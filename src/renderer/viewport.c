/** @file viewport.c
 */

#include "renderer/viewport.h"
#include <vulkan/vulkan_core.h>

int
viewport_new (viewport_t **new_vp, const struct viewport_config *config)
{
  return 0;
}

void
viewport_delete (viewport_t *vp)
{
}

VkSwapchainKHR
viewport_get_swapchain (viewport_t *vp)
{
  return VK_NULL_HANDLE;
}
