/** @file debug_frame_data.h
 */

#pragma once

#include "gpu/gpu_vector.h"

struct debug_frame_data
{
  gpu_vector_t *vertices;
  size_t vertex_num;

  gpu_vector_t *indices;
  size_t index_num;
};
