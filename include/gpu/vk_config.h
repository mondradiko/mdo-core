/** @file vk_config.h
 */

#pragma once

#include <stdint.h> /* for uint32_t */

struct vk_config_t
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
};
