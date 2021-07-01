/** @file gpu_vector.h
 */

#pragma once

#include "gpu_device.h"

/** @typedef gpu_vector_t
 */
typedef struct gpu_vector_s gpu_vector_t;

/** @function gpu_vector_new
 */
int gpu_vector_new (gpu_vector_t **, gpu_device_t *, VkBufferUsageFlags);

/** @function gpu_vector_delete
 */
void gpu_vector_delete (gpu_vector_t *);

/** @function gpu_vector_size
 */
size_t gpu_vector_size (gpu_vector_t *);

/** @function gpu_vector_reserve
 */
int gpu_vector_reserve (gpu_vector_t *, size_t);

/** @function gpu_vector_write
 */
int gpu_vector_write (gpu_vector_t *, const void *, size_t, size_t);

/** @function gpu_vector_get
 */
VkBuffer gpu_vector_get (gpu_vector_t *);
