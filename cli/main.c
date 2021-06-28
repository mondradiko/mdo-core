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

  if (dp)
    sdl_display_delete (dp);

  return 0;
}
