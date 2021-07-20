/** @file openxr_display.h
 */

#pragma once

#include "displays/display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "renderer/camera.h"

/** @typedef openxr_display_t
 */
typedef struct openxr_display_s openxr_display_t;

/** @function openxr_display_new
 */
int openxr_display_new (openxr_display_t **);

/** @function openxr_display_delete
 */
void openxr_display_delete (openxr_display_t *);

/** @function openxr_display_vk_config
 */
void openxr_display_vk_config (openxr_display_t *, vk_config_t *);

/** @function openxr_display_get_camera
 */
camera_t *openxr_display_get_camera (openxr_display_t *);

/** @function openxr_display_begin_session
 */
int openxr_display_begin_session (openxr_display_t *, gpu_device_t *);

/** @function openxr_display_end_session
 */
void openxr_display_end_session (openxr_display_t *);

/** @function openxr_display_poll
 */
void openxr_display_poll (openxr_display_t *, struct display_poll_t *);
