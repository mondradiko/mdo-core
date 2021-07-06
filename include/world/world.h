/** @file world.h
 */

#pragma once

#include "renderer/debug/debug_draw.h"

/** @typedef world_t
 */
typedef struct world_s world_t;

/** @function world_new
 */
int world_new (world_t **, debug_draw_list_t *);

/** @function world_delete
 */
void world_delete (world_t *);

/** @function world_step
 */
void world_step (world_t *, float);
