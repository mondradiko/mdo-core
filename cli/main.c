#include <stdio.h>

#include "displays/sdl/sdl_display.h"

int
main ()
{
  sdl_display_t *dp = NULL;
  if (sdl_display_new (&dp))
    {
      /* TODO(marceline-cramer): custom logging */
      fprintf (stderr, "failed to create SDL display\n");
    }
  else
    {
      struct display_poll_t poll;
      poll.should_exit = 0;
      while (!poll.should_exit)
        {
          sdl_display_poll (dp, &poll);
        }
    }

  if (dp)
    sdl_display_delete (dp);

  return 0;
}
