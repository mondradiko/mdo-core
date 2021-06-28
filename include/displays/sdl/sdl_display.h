/** @file sdl_display.h
 */

#pragma once

#include "displays/display.h"

/** @typedef sdl_display_t
 */
typedef struct sdl_display_s sdl_display_t;

/** @function sdl_display_new
 * @param new_sdl_display
 * @return Zero on success.
 */
int sdl_display_new(sdl_display_t **);

/** @function sdl_display_delete
 */
void sdl_display_delete(sdl_display_t *);

/** @function sdl_display_poll
 * @param dp
 * @param poll
 */
void sdl_display_poll (sdl_display_t *, struct display_poll_t *);
