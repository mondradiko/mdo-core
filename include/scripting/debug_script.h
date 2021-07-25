/** @file debug_script.h
 */

#pragma once

#include "renderer/debug/debug_draw.h"

/** @typedef debug_script_t
 */
typedef struct debug_script_s debug_script_t;

/** @function debug_script_new
 */
int debug_script_new (debug_script_t **, debug_draw_list_t *);

/** @function debug_script_delete
 */
void debug_script_delete (debug_script_t *);

/** @function debug_script_step
 */
void debug_script_step (debug_script_t *, float);