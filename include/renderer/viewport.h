/** @file viewport.h
 */

#pragma once

#include "gpu/gpu_device.h"

#include <vulkan/vulkan_core.h> /* for VkSurfaceKHR, VkSwapchainKHR */

#include "renderer/viewport_uniform.h"

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
  gpu_device_t *gpu;
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
int viewport_new (viewport_t **, VkRenderPass, const struct viewport_config *);

/** @function viewport_delete
 */
void viewport_delete (viewport_t *);

/** @function viewport_write_uniform
 */
void viewport_write_uniform (viewport_t *, viewport_uniform_t *);

/** @function viewport_acquire
 * @param Non-zero if the swapchain has been acquired.
 */
int viewport_acquire (viewport_t *);

/** @function viewport_get_swapchain
 */
VkSwapchainKHR viewport_get_swapchain (viewport_t *);

/** @function viewport_get_on_acquire
 */
VkSemaphore viewport_get_on_acquire (viewport_t *);

/** @function viewport_get_image_index
 */
int viewport_get_image_index (viewport_t *);

/** @function viewport_begin_render_pass
 */
void viewport_begin_render_pass (viewport_t *, VkCommandBuffer);
