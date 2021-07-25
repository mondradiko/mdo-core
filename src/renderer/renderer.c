/** @file renderer.c
 */

#include "renderer/renderer.h"

/* TODO(marceline-cramer): mdo_allocator */
#include <stdlib.h> /* for mem alloc */

#include <TracyC.h>
#include <vulkan/vulkan_core.h>

#include "gpu/gpu_device.h"
#include "log.h"
#include "renderer/debug/debug_pass.h"
#include "renderer/frame_data.h"
#include "renderer/render_phases.h"
#include "renderer/viewport_uniform.h"

#define MAX_CAMERA_NUM 1024
#define MAX_VIEWPORT_NUM (MAX_CAMERA_NUM * MAX_VIEWPORTS_PER_CAMERA)

struct renderer_s
{
  gpu_device_t *gpu;
  VkDevice vkd;
  VkQueue present_queue;

  VkDescriptorSetLayout viewport_layout;

  debug_pass_t *debug_pass;

  struct frame_data frames[MAX_FRAMES_IN_FLIGHT];
  int frame_num;
  int frame_index;
};

static int
create_viewport_layout (renderer_t *ren)
{
  VkDescriptorSetLayoutBinding binding = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  VkDescriptorSetLayoutCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &binding,
  };

  if (vkCreateDescriptorSetLayout (ren->vkd, &ci, NULL, &ren->viewport_layout)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create viewport descriptor set layout\n");
      return 1;
    }

  return 0;
}

static int
frame_data_init (renderer_t *ren, struct frame_data *frame)
{
  frame->command_pool = VK_NULL_HANDLE;
  frame->on_finished = VK_NULL_HANDLE;
  frame->is_in_flight = VK_NULL_HANDLE;
  frame->viewport_buf = NULL;
  frame->descriptor_pool = VK_NULL_HANDLE;

  VkCommandPoolCreateInfo cp_ci = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = gpu_device_gfx_family (ren->gpu),
  };

  if (vkCreateCommandPool (ren->vkd, &cp_ci, NULL, &frame->command_pool)
      != VK_SUCCESS)
    {
      LOG_ERR ("failed to create command pool");
      return 1;
    }

  VkSemaphoreCreateInfo semaphore_ci = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  if (vkCreateSemaphore (ren->vkd, &semaphore_ci, NULL, &frame->on_finished)
      != VK_SUCCESS)
    {
      LOG_ERR ("failed to create semaphore");
      return 1;
    }

  VkFenceCreateInfo fence_ci = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  if (vkCreateFence (ren->vkd, &fence_ci, NULL, &frame->is_in_flight)
      != VK_SUCCESS)
    {
      LOG_ERR ("failed to create fence");
      return 1;
    }

  VkBufferUsageFlags viewport_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (gpu_vector_new (&frame->viewport_buf, ren->gpu, viewport_usage))
    {
      fprintf (stderr, "failed to create viewport buffer\n");
      return 1;
    }

  VkDescriptorPoolSize pool_sizes[1];

  pool_sizes[0] = (VkDescriptorPoolSize){
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 100, /* picked arbitrarily */
  };

  VkDescriptorPoolCreateInfo dp_ci = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .maxSets = 1000, /* arbitrarily picked */
    .poolSizeCount = 1,
    .pPoolSizes = pool_sizes,
  };

  if (vkCreateDescriptorPool (ren->vkd, &dp_ci, NULL, &frame->descriptor_pool)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create descriptor pool\n");
      return 1;
    }

  return 0;
}

static void
frame_data_cleanup (renderer_t *ren, struct frame_data *frame)
{
  if (frame->descriptor_pool)
    vkDestroyDescriptorPool (ren->vkd, frame->descriptor_pool, NULL);

  if (frame->viewport_buf)
    gpu_vector_delete (frame->viewport_buf);

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
  ren->viewport_layout = VK_NULL_HANDLE;
  ren->debug_pass = NULL;
  ren->frame_index = 0;
  ren->frame_num = 0;

  int gfx_family = gpu_device_gfx_family (gpu);
  int queue_index = 0;
  vkGetDeviceQueue (ren->vkd, gfx_family, queue_index, &ren->present_queue);

  if (create_viewport_layout (ren))
    return 1;

  if (debug_pass_new (&ren->debug_pass, ren, rp))
    {
      LOG_ERR ("failed to create debug pass");
      return 1;
    }

  for (int i = 0; i < 2; i++)
    {
      ren->frame_num++;
      struct frame_data *frame = &ren->frames[i];

      if (frame_data_init (ren, frame))
        {
          LOG_ERR ("failed to create frame data");
          return 1;
        }

      if (debug_frame_data_init (ren->debug_pass, &frame->debug))
        {
          LOG_ERR ("failed to create debug frame data");
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

  if (ren->viewport_layout)
    vkDestroyDescriptorSetLayout (ren->vkd, ren->viewport_layout, NULL);

  free (ren);
}

gpu_device_t *
renderer_get_gpu (renderer_t *ren)
{
  return ren->gpu;
}

debug_draw_list_t *
renderer_get_debug_draw_list (renderer_t *ren)
{
  return debug_pass_get_draw_list (ren->debug_pass);
}

VkDescriptorSetLayout
renderer_get_viewport_layout (renderer_t *ren)
{
  return ren->viewport_layout;
}

void
renderer_render_frame (renderer_t *ren, camera_t **cameras, int camera_num)
{
  if (camera_num > MAX_CAMERA_NUM)
    {
      LOG_ERR ("too many cameras (how did you even do that?)");
      return;
    }

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

  int acquired_num = 0;
  for (int i = 0; i < viewport_num; i++)
    {
      if (viewport_acquire (viewports[i]))
        {
          viewports[acquired_num] = viewports[i];
          acquired_num++;
        }
    }

  /* cull out unacquired viewports */
  viewport_num = acquired_num;

  if (viewport_num == 0)
    return;

  ren->frame_index++;
  if (ren->frame_index >= ren->frame_num)
    ren->frame_index = 0;

  struct frame_data *frame = &ren->frames[ren->frame_index];

  vkWaitForFences (ren->vkd, 1, &frame->is_in_flight, VK_TRUE, UINT64_MAX);
  vkResetFences (ren->vkd, 1, &frame->is_in_flight);
  vkResetCommandPool (ren->vkd, frame->command_pool, 0);
  vkResetDescriptorPool (ren->vkd, frame->descriptor_pool, 0);

  viewport_uniform_t viewport_uniforms[MAX_VIEWPORT_NUM];
  for (int i = 0; i < viewport_num; i++)
    {
      viewport_write_uniform (viewports[i], &viewport_uniforms[i]);
    }

  gpu_vector_write (frame->viewport_buf, viewport_uniforms, viewport_num,
                    sizeof (viewport_uniform_t));

  VkDescriptorSetAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = frame->descriptor_pool,
    .descriptorSetCount = 1,
    .pSetLayouts = &ren->viewport_layout,
  };

  vkAllocateDescriptorSets (ren->vkd, &alloc_info, &frame->viewport_set);

  VkDescriptorBufferInfo vp_buf = {
    .buffer = gpu_vector_get (frame->viewport_buf),
    .offset = 0,
    /*.range = gpu_vector_size (frame->viewport_buf),*/
    .range = sizeof (viewport_uniform_t),
  };

  VkWriteDescriptorSet write_info = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .dstSet = frame->viewport_set,
    .dstBinding = 0,
    .descriptorCount = 1,
    .pBufferInfo = &vp_buf,
  };

  vkUpdateDescriptorSets (ren->vkd, 1, &write_info, 0, NULL);

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

  VkCommandBufferAllocateInfo cmd_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = frame->command_pool,
    .commandBufferCount = 1,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };

  vkAllocateCommandBuffers (ren->vkd, &cmd_info, &cmd);

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
        .viewport_index = i,
        .viewport_set = frame->viewport_set,
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
          LOG_ERR ("failed to present to swapchains");
        }
    }

  TracyCFrameMarkNamed ("render");
}
