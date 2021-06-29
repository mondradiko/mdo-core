/** @file camera.h
 */

#pragma once

#include "gpu/gpu_device.h"
#include "renderer/viewport.h"

#define MAX_VIEWPORTS_PER_CAMERA 8

/** @typedef camera_t
 */
typedef struct camera_s camera_t;

struct camera_config
{
  gpu_device_t *gpu;
  const struct viewport_config *viewport_configs;
  int viewport_num;
};

/** @function camera_new
 */
int camera_new (camera_t **, const struct camera_config *);

/** @function camera_delete
 */
void camera_delete (camera_t *);

/** @function camera_acquire
 * @return the number of viewports acquired.
 */
int camera_acquire (camera_t *, viewport_t **);
