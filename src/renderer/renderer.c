/** @file renderer.c
 */

#include "renderer/renderer.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): mdo_allocator */
#include <stdlib.h> /* for mem alloc */
#include <vulkan/vulkan_core.h>

#include "gpu/gpu_device.h"
#include "renderer/render_phases.h"

#define MAX_CAMERA_NUM 1024
#define MAX_VIEWPORT_NUM (MAX_CAMERA_NUM * MAX_VIEWPORTS_PER_CAMERA)

struct frame_data
{
  VkCommandPool command_pool;
};

struct renderer_s
{
  gpu_device_t *gpu;
  VkDevice vkd;
  VkQueue present_queue;
  struct frame_data frames[MAX_FRAMES_IN_FLIGHT];
  int frame_num;
};

static int
frame_data_init (renderer_t *ren, struct frame_data *frame)
{
  frame->command_pool = VK_NULL_HANDLE;

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

  return 0;
}

static void
frame_data_cleanup (renderer_t *ren, struct frame_data *frame)
{
  if (frame->command_pool)
    vkDestroyCommandPool (ren->vkd, frame->command_pool, NULL);
}

int
renderer_new (renderer_t **new_ren, gpu_device_t *gpu)
{
  renderer_t *ren = malloc (sizeof (renderer_t));
  *new_ren = ren;

  ren->gpu = gpu;
  ren->vkd = gpu_device_get (gpu);

  int gfx_family = gpu_device_gfx_family (gpu);
  int queue_index = 0;
  vkGetDeviceQueue (ren->vkd, gfx_family, queue_index, &ren->present_queue);

  ren->frame_num = 2;
  for (int i = 0; i < ren->frame_num; i++)
    frame_data_init (ren, &ren->frames[i]);

  return 0;
}

void
renderer_delete (renderer_t *ren)
{
  for (int i = 0; i < ren->frame_num; i++)
    frame_data_cleanup (ren, &ren->frames[i]);

  free (ren);
}

void
renderer_render_frame (renderer_t *ren, camera_t **cameras, int camera_num)
{
  if (camera_num > MAX_CAMERA_NUM)
    {
      fprintf (stderr, "too many cameras (how did you even do that?)\n");
      return;
    }

  int viewport_num = 0;
  viewport_t *viewports[MAX_VIEWPORT_NUM];
  for (int i = 0; i < camera_num; i++)
    {
      int acquired_num = camera_acquire (cameras[i], &viewports[viewport_num]);
      viewport_num += acquired_num;
    }

  for (int i = 0; i < viewport_num; i++)
      viewport_acquire (viewports[i]);

  int swapchain_num = 0;
  VkSwapchainKHR swapchains[MAX_VIEWPORT_NUM];

  int wait_semaphore_num = 0;
  VkSemaphore wait_semaphores[MAX_VIEWPORT_NUM];

  uint32_t image_indices[MAX_VIEWPORT_NUM];

  for (int i = 0; i < viewport_num; i++)
    {
      VkSwapchainKHR swapchain = viewport_get_swapchain (viewports[i]);
      if (swapchain != VK_NULL_HANDLE)
        {
          VkSemaphore wait_semaphore = viewport_get_on_acquire (viewports[i]);
          if (wait_semaphore != VK_NULL_HANDLE)
            wait_semaphores[wait_semaphore_num++] = wait_semaphore;

          image_indices[swapchain_num]
              = viewport_get_image_index (viewports[i]);
          swapchains[swapchain_num] = swapchain;
          swapchain_num++;
        }
    }

  if (swapchain_num > 0)
    {
      VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = wait_semaphore_num,
        .pWaitSemaphores = wait_semaphores,
        .swapchainCount = swapchain_num,
        .pSwapchains = swapchains,
        .pImageIndices = image_indices,
      };

      if (vkQueuePresentKHR (ren->present_queue, &present_info) != VK_SUCCESS)
        {
          fprintf (stderr, "failed to present to swapchains\n");
        }
    }
}
