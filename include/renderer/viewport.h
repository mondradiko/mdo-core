/** @file viewport.h
 */

#pragma once

#include <vulkan/vulkan_core.h> /* for VkSurfaceKHR, VkSwapchainKHR */

/** @typedef viewport_t
 */
typedef struct viewport_s viewport_t;

enum viewport_type
{
  VIEWPORT_TYPE_SURFACE,
};

struct viewport_surface_config
{
  VkSurfaceKHR surface;
};

struct viewport_config
{
  enum viewport_type type;
  int width;
  int height;

  union
  {
    struct viewport_surface_config surface;
  } sub;
};

/** @function viewport_new
 */
int viewport_new (viewport_t **, const struct viewport_config *);

/** @function viewport_delete
 */
void viewport_delete (viewport_t *);

/** @function viewport_get_swapchain
 */
VkSwapchainKHR viewport_get_swapchain (viewport_t *);

/** @function viewport_get_image_index
 */
int viewport_get_image_index (viewport_t *);
