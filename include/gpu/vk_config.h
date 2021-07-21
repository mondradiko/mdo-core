/** @file vk_config.h
 */

#pragma once

#include <stdint.h> /* for uint32_t */

#include <vulkan/vulkan_core.h> /* for VkPhysicalDevice */

typedef struct
{
  /**
   * The minimum/maximum Vulkan API version. Set to one of VK_API_VERSION_*.
   */
  uint32_t min_api_version;
  uint32_t max_api_version;

  /**
   * A list of required instance/device extensions. The list is a string, where
   * each element is separated by a space.
   *
   * The provider of this list is reponsible for freeing its memory.
   */
  const char *instance_extensions;
  const char *device_extensions;

  /** @function select_physical_device_cb
   * If the callback pointer is NULL, autoselects.
   * If vkpd is set to NULL, autoselects.
   * Returns 0 on success.
   */
  int (*select_physical_device_cb) (void *, VkInstance, VkPhysicalDevice *);

  /**
   * Userdata to pass to #select_physical_device_cb.
   */
  void *data;
} vk_config_t;
