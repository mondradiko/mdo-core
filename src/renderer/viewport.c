/** @file viewport.c
 */

#include "renderer/viewport.h"

#include "gpu/gpu_device.h"
#include "renderer/render_phases.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): mdo_allocator */
#include <stdlib.h> /* for mem alloc */

#include <vulkan/vulkan_core.h>

#define MAX_IMAGE_NUM 8

struct vp_image
{
};

struct viewport_s
{
  gpu_device_t *gpu;
  VkDevice vkd;
  VkSwapchainKHR swapchain;

  struct vp_image images[MAX_IMAGE_NUM];
  int image_index;

  VkSemaphore on_image_acquire[MAX_FRAMES_IN_FLIGHT];
  int image_acquire_index;
};

static int
surface_init (viewport_t *vp, const struct viewport_config *config)
{
  fprintf (stderr, "creating surface-based viewport\n");

  VkPhysicalDevice vkpd = gpu_device_get_physical (vp->gpu);
  int gfx_family = gpu_device_gfx_family (vp->gpu);
  VkSurfaceKHR surface = config->sub.surface.surface;
  VkBool32 supported;
  vkGetPhysicalDeviceSurfaceSupportKHR (vkpd, gfx_family, surface, &supported);
  if (supported != VK_TRUE)
    {
      fprintf (stderr, "surface is unsupported on this queue family\n");
      return 1;
    }

  VkSurfaceCapabilitiesKHR caps;
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR (vkpd, surface, &caps)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to get surface capabilities\n");
      return 1;
    }

  VkSwapchainCreateInfoKHR ci = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface,
    .minImageCount = caps.minImageCount,

    /* TODO(marceline-cramer): autoselect */
    .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
    .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,

    .imageExtent = {
      .width = config->width,
      .height = config->height,
    },

    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = caps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

    /* TODO(marceline-cramer): autoselect */
    .presentMode = VK_PRESENT_MODE_FIFO_KHR,

    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  if (vkCreateSwapchainKHR (vp->vkd, &ci, NULL, &vp->swapchain) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create swapchain\n");
      return 1;
    }

  return 0;
}

int
create_semaphores (viewport_t *vp)
{
  VkSemaphoreCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if (vkCreateSemaphore (vp->vkd, &ci, NULL, &vp->on_image_acquire[i])
          != VK_SUCCESS)
        {
          fprintf (stderr, "failed to create image acquisition semaphore\n");
          return 1;
        }
    }

  return 0;
}

int
viewport_new (viewport_t **new_vp, const struct viewport_config *config)
{
  viewport_t *vp = malloc (sizeof (viewport_t));
  *new_vp = vp;

  vp->gpu = config->gpu;
  vp->vkd = gpu_device_get (vp->gpu);
  vp->swapchain = VK_NULL_HANDLE;

  switch (config->type)
    {
    case VIEWPORT_TYPE_SURFACE:
      {
        if (surface_init (vp, config))
          return 1;
        break;
      }
    default:
      {
        fprintf (stderr, "unrecognized viewport type\n");
        return 1;
      }
    }

  if (create_semaphores (vp))
    return 1;

  return 0;
}

void
viewport_delete (viewport_t *vp)
{
  if (vp->swapchain)
    vkDestroySwapchainKHR (vp->vkd, vp->swapchain, NULL);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    vkDestroySemaphore (vp->vkd, vp->on_image_acquire[i], NULL);

  free (vp);
}

int
viewport_acquire (viewport_t *vp)
{
  vp->image_acquire_index++;
  if (vp->image_acquire_index >= MAX_FRAMES_IN_FLIGHT)
    vp->image_acquire_index = 0;
  VkSemaphore on_acquire = vp->on_image_acquire[vp->image_acquire_index];

  uint32_t image_index;
  vkAcquireNextImageKHR (vp->vkd, vp->swapchain, UINT64_MAX, on_acquire,
                         VK_NULL_HANDLE, &image_index);

  vp->image_index = image_index;

  return 1;
}

VkSwapchainKHR
viewport_get_swapchain (viewport_t *vp)
{
  return vp->swapchain;
}

VkSemaphore
viewport_get_on_acquire (viewport_t *vp)
{
  return vp->on_image_acquire [vp->image_acquire_index];
}

int
viewport_get_image_index (viewport_t *vp)
{
  return vp->image_index;
}
