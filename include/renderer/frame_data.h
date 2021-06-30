/** @file frame_data.h
 */

#pragma once

#include <vulkan/vulkan.h>

#include "renderer/debug/debug_frame_data.h"

struct frame_data
{
  VkCommandPool command_pool;
  VkSemaphore on_finished;
  VkFence is_in_flight;

  struct debug_frame_data debug;
};
