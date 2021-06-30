/** @file gpu_shader.h
 */

#pragma once

#include "gpu/gpu_device.h"

/** @typedef gpu_shader_t
 */
typedef struct gpu_shader_s gpu_shader_t;

/** @function gpu_shader_new
 */
int gpu_shader_new (gpu_shader_t **, gpu_device_t *);

/** @function gpu_shader_delete
 */
void gpu_shader_delete (gpu_shader_t *);

/** @function gpu_shader_load_from_file
 */
int gpu_shader_load_from_file (gpu_shader_t *, const char *);

/** @function gpu_shader_get
 */
int gpu_shader_get (gpu_shader_t *, VkPipelineShaderStageCreateInfo *);
