/** @file renderer.c
 */

#include "renderer/renderer.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): mdo_allocator */
#include <stdlib.h> /* for mem alloc */

#include <TracyC.h>
#include <vulkan/vulkan_core.h>

#include "gpu/gpu_device.h"
#include "renderer/debug/debug_pass.h"
#include "renderer/frame_data.h"
#include "renderer/render_phases.h"

#define MAX_CAMERA_NUM 1024
#define MAX_VIEWPORT_NUM (MAX_CAMERA_NUM * MAX_VIEWPORTS_PER_CAMERA)

struct renderer_s
{
  gpu_device_t *gpu;
  VkDevice vkd;
  VkQueue present_queue;

  debug_pass_t *debug_pass;

  struct frame_data frames[MAX_FRAMES_IN_FLIGHT];
  int frame_num;
  int frame_index;
};

static int
frame_data_init (renderer_t *ren, struct frame_data *frame)
{
  frame->command_pool = VK_NULL_HANDLE;
  frame->on_finished = VK_NULL_HANDLE;
  frame->is_in_flight = VK_NULL_HANDLE;

  VkCommandPoolCreateInfo cp_ci = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = gpu_device_gfx_family (ren->gpu),
  };

  if (vkCreateCommandPool (ren->vkd, &cp_ci, NULL, &frame->command_pool)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create command pool\n");
      return 1;
    }

  VkSemaphoreCreateInfo semaphore_ci = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  if (vkCreateSemaphore (ren->vkd, &semaphore_ci, NULL, &frame->on_finished)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create semaphore\n");
      return 1;
    }

  VkFenceCreateInfo fence_ci = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  if (vkCreateFence (ren->vkd, &fence_ci, NULL, &frame->is_in_flight)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create fence\n");
      return 1;
    }

  return 0;
}

static void
frame_data_cleanup (renderer_t *ren, struct frame_data *frame)
{
  if (frame->command_pool)
    vkDestroyCommandPool (ren->vkd, frame->command_pool, NULL);

  if (frame->on_finished)
    vkDestroySemaphore (ren->vkd, frame->on_finished, NULL);

  if (frame->is_in_flight)
    vkDestroyFence (ren->vkd, frame->is_in_flight, NULL);
}

int
renderer_new (renderer_t **new_ren, gpu_device_t *gpu, VkRenderPass rp)
{
  renderer_t *ren = malloc (sizeof (renderer_t));
  *new_ren = ren;

  ren->gpu = gpu;
  ren->vkd = gpu_device_get (gpu);
  ren->debug_pass = NULL;
  ren->frame_index = 0;

  int gfx_family = gpu_device_gfx_family (gpu);
  int queue_index = 0;
  vkGetDeviceQueue (ren->vkd, gfx_family, queue_index, &ren->present_queue);

  if (debug_pass_new (&ren->debug_pass, ren, rp))
    {
      fprintf (stderr, "failed to create debug pass\n");
      return 1;
    }

  for (int i = 0; i < 2; i++)
    {
      ren->frame_num++;
      struct frame_data *frame = &ren->frames[i];

      if (frame_data_init (ren, frame))
        {
          fprintf (stderr, "failed to create frame data\n");
          return 1;
        }

      if (debug_frame_data_init (ren->debug_pass, &frame->debug))
        {
          fprintf (stderr, "failed to create debug frame data\n");
          return 1;
        }
    }

  return 0;
}

void
renderer_delete (renderer_t *ren)
{
  vkQueueWaitIdle (ren->present_queue);

  for (int i = 0; i < ren->frame_num; i++)
    {
      struct frame_data *frame = &ren->frames[i];
      debug_frame_data_cleanup (ren->debug_pass, &frame->debug);
      frame_data_cleanup (ren, frame);
    }

  debug_pass_delete (ren->debug_pass);

  free (ren);
}

gpu_device_t *
renderer_get_gpu (renderer_t *ren)
{
  return ren->gpu;
}

static const char *RENDER_FRAME_NAME = "render";

void
renderer_render_frame (renderer_t *ren, camera_t **cameras, int camera_num)
{
  if (camera_num > MAX_CAMERA_NUM)
    {
      fprintf (stderr, "too many cameras (how did you even do that?)\n");
      return;
    }

  ren->frame_index++;
  if (ren->frame_index >= ren->frame_num)
    ren->frame_index = 0;

  struct frame_data *frame = &ren->frames[ren->frame_index];

  vkWaitForFences (ren->vkd, 1, &frame->is_in_flight, VK_TRUE, UINT64_MAX);
  vkResetFences (ren->vkd, 1, &frame->is_in_flight);
  vkResetCommandPool (ren->vkd, frame->command_pool, 0);

  TracyCFrameMarkStart(RENDER_FRAME_NAME);

  int viewport_num = 0;
  viewport_t *viewports[MAX_VIEWPORT_NUM];
  camera_t *viewport_cameras[MAX_VIEWPORT_NUM];
  for (int i = 0; i < camera_num; i++)
    {
      int acquired_num = camera_acquire (cameras[i], &viewports[viewport_num]);

      for (int j = 0; j < acquired_num; j++)
        viewport_cameras[j + viewport_num] = cameras[j];

      viewport_num += acquired_num;
    }

  for (int i = 0; i < viewport_num; i++)
    viewport_acquire (viewports[i]);

  int swapchain_num = 0;
  VkSwapchainKHR swapchains[MAX_VIEWPORT_NUM];

  int wait_semaphore_num = 0;
  VkSemaphore wait_semaphores[MAX_VIEWPORT_NUM];
  VkPipelineStageFlags wait_stage_flags[MAX_VIEWPORT_NUM];

  uint32_t image_indices[MAX_VIEWPORT_NUM];

  for (int i = 0; i < viewport_num; i++)
    {
      VkSwapchainKHR swapchain = viewport_get_swapchain (viewports[i]);
      if (swapchain != VK_NULL_HANDLE)
        {
          VkSemaphore wait_semaphore = viewport_get_on_acquire (viewports[i]);
          if (wait_semaphore != VK_NULL_HANDLE)
            {
              wait_stage_flags[wait_semaphore_num]
                  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
              wait_semaphores[wait_semaphore_num] = wait_semaphore;
              wait_semaphore_num++;
            }

          image_indices[swapchain_num]
              = viewport_get_image_index (viewports[i]);
          swapchains[swapchain_num] = swapchain;
          swapchain_num++;
        }
    }

  VkCommandBuffer cmd;

  VkCommandBufferAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = frame->command_pool,
    .commandBufferCount = 1,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };

  vkAllocateCommandBuffers (ren->vkd, &alloc_info, &cmd);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer (cmd, &begin_info);

  for (int i = 0; i < viewport_num; i++)
    {
      viewport_begin_render_pass (viewports[i], cmd);

      const struct render_context ctx = {
        .cmd = cmd,
        .camera = viewport_cameras[i],
      };

      debug_pass_render (ren->debug_pass, &ctx, &frame->debug);

      vkCmdEndRenderPass (cmd);
    }

  vkEndCommandBuffer (cmd);

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = wait_semaphore_num,
    .pWaitSemaphores = wait_semaphores,
    .pWaitDstStageMask = wait_stage_flags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &frame->on_finished,
  };

  vkQueueSubmit (ren->present_queue, 1, &submit_info, frame->is_in_flight);

  if (swapchain_num > 0)
    {
      VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->on_finished,
        .swapchainCount = swapchain_num,
        .pSwapchains = swapchains,
        .pImageIndices = image_indices,
      };

      if (vkQueuePresentKHR (ren->present_queue, &present_info) != VK_SUCCESS)
        {
          fprintf (stderr, "failed to present to swapchains\n");
        }
    }

  TracyCFrameMarkEnd (RENDER_FRAME_NAME);
}
