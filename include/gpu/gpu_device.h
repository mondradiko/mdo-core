/** @file gpu_device.h
 */

#pragma once

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
