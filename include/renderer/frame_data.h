/** @file frame_data.h
 */

#pragma once

#include "gpu/gpu_vector.h"
#include "renderer/debug/debug_frame_data.h"

struct frame_data
{
  /* commands and synchronization */
  VkCommandPool command_pool;
  VkSemaphore on_finished;
  VkFence is_in_flight;

  /* global GPU data */
  gpu_vector_t *viewport_buf;

  /* descriptors */
  VkDescriptorPool descriptor_pool;
  VkDescriptorSet viewport_set;

  /* per-pass frame data */
  struct debug_frame_data debug;
};
