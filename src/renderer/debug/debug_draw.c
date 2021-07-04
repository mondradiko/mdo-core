/** @file debug_draw.c
 */

#include "renderer/debug/debug_draw.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

struct debug_draw_list_s
{
  struct {
    debug_draw_vertex_t *vals;
    size_t num;
    size_t capacity;
  } vertices;

  struct {
    debug_draw_index_t *vals;
    size_t num;
    size_t capacity;
  } indices;
};

int
debug_draw_list_new (debug_draw_list_t **new_ddl)
{
  debug_draw_list_t *ddl = malloc (sizeof (debug_draw_list_t));
  *new_ddl = ddl;

  const int CAPACITY = 1024;

  ddl->vertices.num = 0;
  ddl->vertices.capacity = CAPACITY;
  ddl->vertices.vals= calloc (CAPACITY, sizeof (debug_draw_vertex_t));

  ddl->indices.num = 0;
  ddl->indices.capacity = CAPACITY;
  ddl->indices.vals = calloc (CAPACITY, sizeof (debug_draw_index_t));

  return 0;
}

void
debug_draw_list_delete (debug_draw_list_t *ddl)
{
  if (ddl->vertices.vals)
    free (ddl->vertices.vals);

  if (ddl->indices.vals)
    free (ddl->indices.vals);

  free (ddl);
}

void
debug_draw_list_clear (debug_draw_list_t *ddl)
{
  ddl->vertices.num = 0;
  ddl->indices.num = 0;
}

debug_draw_index_t
debug_draw_list_vertex (debug_draw_list_t *ddl,
                        const debug_draw_vertex_t *vertex)
{
  debug_draw_index_t index = ddl->vertices.num;
  size_t required_num = ddl->vertices.num + 1;

  if (ddl->vertices.capacity < required_num) {
    ddl->vertices.capacity *= 2;
    size_t required_size = required_num * sizeof (debug_draw_vertex_t);
    ddl->vertices.vals = realloc (ddl->vertices.vals, required_size);
  }

  ddl->vertices.vals[index] = *vertex;
  ddl->vertices.num++;

  return index;
}

void
debug_draw_list_line (debug_draw_list_t *ddl, debug_draw_index_t vertex1,
                      debug_draw_index_t vertex2)
{
  size_t first_index = ddl->indices.num;
  size_t required_num = ddl->indices.num + 2;

  if (ddl->indices.capacity < required_num) {
    ddl->indices.capacity *= 2;
    size_t required_size = required_num * sizeof (debug_draw_index_t);
    ddl->indices.vals = realloc (ddl->indices.vals, required_size);
  }

  ddl->indices.vals[first_index + 0] = vertex1;
  ddl->indices.vals[first_index + 1] = vertex2;
  ddl->indices.num += 2;
}

const debug_draw_vertex_t *
debug_draw_list_vertices (debug_draw_list_t *ddl)
{
  return ddl->vertices.vals;
}

size_t
debug_draw_list_vertex_num (debug_draw_list_t *ddl)
{
  return ddl->vertices.num;
}

const debug_draw_index_t *
debug_draw_list_indices (debug_draw_list_t *ddl)
{
  return ddl->indices.vals;
}

size_t
debug_draw_list_index_num (debug_draw_list_t *ddl)
{
  return ddl->vertices.num;
}
