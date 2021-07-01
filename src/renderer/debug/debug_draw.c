/** @file debug_draw.c
 */

#include "renderer/debug/debug_draw.h"

int
debug_draw_list_new (debug_draw_list_t **new_ddl)
{
  return 0;
}

void
debug_draw_list_delete (debug_draw_list_t *ddl)
{
}

void
debug_draw_list_clear (debug_draw_list_t *ddl)
{
}

debug_draw_index_t
debug_draw_list_vertex (debug_draw_list_t *ddl,
                        const debug_draw_vertex_t *vertex)
{
  return 0;
}

void
debug_draw_list_line (debug_draw_list_t *ddl, debug_draw_index_t vertex1,
                      debug_draw_index_t vertex2)
{
}

const debug_draw_vertex_t *
debug_draw_list_vertices (debug_draw_list_t *ddl)
{
  return NULL;
}

size_t
debug_draw_list_vertex_num (debug_draw_list_t *ddl)
{
  return 0;
}

const debug_draw_index_t *
debug_draw_list_indices (debug_draw_list_t *ddl)
{
  return NULL;
}

size_t
debug_draw_list_index_num (debug_draw_list_t *ddl)
{
  return 0;
}
