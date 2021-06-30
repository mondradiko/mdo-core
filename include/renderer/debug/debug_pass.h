/** @file debug_pass.h
 */

#pragma once

#include "renderer/renderer.h"
#include "renderer/debug/debug_frame_data.h"

/** @typedef debug_pass_t
 */
typedef struct debug_pass_s debug_pass_t;

/** @function debug_pass_new
 */
int debug_pass_new (debug_pass_t **, renderer_t *);

/** @function debug_pass_delete
 */
void debug_pass_delete (debug_pass_t *);

/** @function debug_frame_data_init
 */
int debug_frame_data_init (debug_pass_t *, struct debug_frame_data *);

/** @function debug_frame_data_cleanup
 */
void debug_frame_data_cleanup (debug_pass_t *, struct debug_frame_data *);
