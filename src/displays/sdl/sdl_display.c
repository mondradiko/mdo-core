/** @file sdl_display.c
 */

#include "displays/sdl/sdl_display.h"

/* TODO(marceline-cramer): replace with mdo_allocator */
#include <SDL2/SDL_error.h>
#include <stdio.h>  /* for fprintf */
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

  dp->window = SDL_CreateWindow ("Mondradiko Core", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, 800, 600,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

  if (!dp->window)
    {
      /* TODO(marceline-cramer): custom logging */
      fprintf (stderr, "failed to create SDL window: %s\n", SDL_GetError ());
      return -1;
    }

  return 0;
}

void
sdl_display_delete (sdl_display_t *dp)
{
  if (dp->window)
    SDL_DestroyWindow (dp->window);

  free (dp);
}

void
sdl_display_poll (sdl_display_t *dp, struct display_poll_t *poll)
{
  poll->should_exit = 0;
  poll->should_render = 1;

  SDL_Event e;
  while (SDL_PollEvent (&e))
    {
      switch (e.type)
        {
        case SDL_QUIT:
          {
            poll->should_exit = 1;
            break;
          }

        default:
          break;
        }
    }
}
