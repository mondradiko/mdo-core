/** @file sdl_display.h
 */

#pragma once

/* forward declarations */
struct display_poll_t;
struct gpu_device_t;
struct vk_config_t;

/** @typedef sdl_display_t
 */
typedef struct sdl_display_s sdl_display_t;

/** @function sdl_display_new
 * @param new_sdl_display
 * @return Zero on success.
 */
int sdl_display_new (sdl_display_t **);

/** @function sdl_display_delete
 */
void sdl_display_delete (sdl_display_t *);

/** @function sdl_display_vk_config
 */
void sdl_display_vk_config (sdl_display_t *, struct vk_config_t *);

/** @function sdl_display_begin_session
 */
int sdl_display_begin_session (sdl_display_t *, struct gpu_device_t *);

/** @function sdl_display_poll
 * @param dp
 * @param poll
 */
void sdl_display_poll (sdl_display_t *, struct display_poll_t *);
