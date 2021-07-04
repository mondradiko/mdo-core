/** @file debug_draw.h
 */

#pragma once

#include <stddef.h> /* for size_t */
#include <stdint.h> /* for uint32_t */

/** @typedef debug_draw_vertex_t
 */
typedef struct debug_draw_vertex_t
{
  float position[3];
  float color[3];
} debug_draw_vertex_t;

/** @typedef debug_draw_index_t
 */
typedef uint32_t debug_draw_index_t;

/** @typedef debug_draw_list_t
 */
typedef struct debug_draw_list_s debug_draw_list_t;

/** @function debug_draw_list_new
 */
int debug_draw_list_new (debug_draw_list_t **);

/** @function debug_draw_list_delete
 */
void debug_draw_list_delete (debug_draw_list_t *);

/** @function debug_draw_list_clear
 */
void debug_draw_list_clear (debug_draw_list_t *);

/** @function debug_draw_list_vertex
 */
debug_draw_index_t debug_draw_list_vertex (debug_draw_list_t *,
                                           const debug_draw_vertex_t *);

/** @function debug_draw_list_line
 */
void debug_draw_list_line (debug_draw_list_t *, debug_draw_index_t,
                           debug_draw_index_t);

/** @function debug_draw_list_vertices
 */
const debug_draw_vertex_t *debug_draw_list_vertices (debug_draw_list_t *);

/** @function debug_draw_list_vertex_num
 */
size_t debug_draw_list_vertex_num (debug_draw_list_t *);

/** @function debug_draw_list_indices
 */
const debug_draw_index_t *debug_draw_list_indices (debug_draw_list_t *);

/** @function debug_draw_list_index_num
 */
size_t debug_draw_list_index_num (debug_draw_list_t *);
