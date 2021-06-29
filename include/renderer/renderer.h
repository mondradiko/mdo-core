/** @file renderer.h
 */

#pragma once

#include "gpu/gpu_device.h"
#include "renderer/camera.h"

/** @typedef renderer_t
 */
typedef struct renderer_s renderer_t;

/** @function renderer_new
 */
int renderer_new (renderer_t **, gpu_device_t *);

/** @function renderer_delete
 */
void renderer_delete (renderer_t *);

/** @function renderer_render_frame
 */
void renderer_render_frame (renderer_t *, camera_t **, int);
