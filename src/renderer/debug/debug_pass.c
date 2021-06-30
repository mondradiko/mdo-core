/** @file debug_pass.c
 */

#include "renderer/debug/debug_pass.h"

int
debug_pass_new (debug_pass_t **new_dbp, renderer_t *ren)
{
  return 0;
}

void
debug_pass_delete (debug_pass_t *dbp)
{
}

int
debug_frame_data_init (debug_pass_t *dbp, struct debug_frame_data *frame)
{
  return 0;
}

void
debug_frame_data_cleanup (debug_pass_t *dbp, struct debug_frame_data *frame)
{
}
