/** @file renderer.h
 */

#pragma once

#include "gpu/gpu_device.h"
#include "renderer/camera.h"

/** @typedef renderer_t
 */
typedef struct renderer_s renderer_t;

/** @function renderer_new
 * TODO(marceline-cramer): The render pass is passed in so that pipelines for
 * each render pass can be created ahead of time. A pipeline creation
 * abstraction object is needed so that renderer render pass coupling can be
 * deferred until #renderer_render_frame, where the renderer actually receives
 * the cameras and by association render passes.
 */
int renderer_new (renderer_t **, gpu_device_t *, VkRenderPass);

/** @function renderer_delete
 */
void renderer_delete (renderer_t *);

/** @function renderer_get_gpu
 */
gpu_device_t *renderer_get_gpu (renderer_t *);

/** @function renderer_render_frame
 */
void renderer_render_frame (renderer_t *, camera_t **, int);
