/** @file sdl_display.c
 */

#include "displays/sdl/sdl_display.h"

/* TODO(marceline-cramer): replace with mdo_allocator */
#include <stdlib.h> /* for mem alloc*/

#include <SDL2/SDL.h>

struct sdl_display_s
{
  SDL_Window *window;
};

int
sdl_display_new (sdl_display_t **new_dp)
{
  sdl_display_t *dp = malloc (sizeof (sdl_display_t));
  *new_dp = dp;

  return 0;
}

void
sdl_display_delete (sdl_display_t *dp)
{
  free (dp);
}
