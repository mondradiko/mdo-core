/** @file gpu_device.h
 */

#pragma once

#include <vulkan/vulkan_core.h> /* for handle types */

/* forward declarations */
struct vk_config_t;

/** @typedef gpu_device_t
 */
typedef struct gpu_device_s gpu_device_t;

/** @function gpu_device_new
 */
int gpu_device_new (gpu_device_t **, const struct vk_config_t *);

/** @function gpu_device_delete
 */
void gpu_device_delete (gpu_device_t *);

/** @function gpu_device_get_instance
 */
VkInstance gpu_device_get_instance (gpu_device_t *);

/** @function gpu_device_get_physical
 */
VkPhysicalDevice gpu_device_get_physical (gpu_device_t *);

/** @function gpu_device_get
 */
VkDevice gpu_device_get (gpu_device_t *);

/** @function gpu_device_gfx_family
 */
int gpu_device_gfx_family (gpu_device_t *);
