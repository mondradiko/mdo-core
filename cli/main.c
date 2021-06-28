#include <stdio.h>

#include "displays/sdl/sdl_display.h"
#include "gpu/gpu_device.h"

typedef struct cli_state_s
{
  sdl_display_t *dp;
  gpu_device_t *gpu;
} cli_state_t;

int
init_cli_state (cli_state_t *cli)
{
  cli->dp = NULL;
  cli->gpu = NULL;

  if (gpu_device_new (&cli->gpu))
    {
      fprintf (stderr, "failed to create GPU device\n");
      return 1;
    }

  if (sdl_display_new (&cli->dp))
    {
      fprintf (stderr, "failed to create SDL display\n");
      return 1;
    }

  return 0;
}

void
cleanup_cli_state (cli_state_t *cli)
{
  if (cli->dp)
    sdl_display_delete (cli->dp);

  if (cli->gpu)
    gpu_device_delete (cli->gpu);
}

int
main ()
{
  cli_state_t cli;

  int result = init_cli_state (&cli);
  if (result)
    return result;

  struct display_poll_t poll;
  poll.should_exit = 0;
  while (!poll.should_exit)
    {
      sdl_display_poll (cli.dp, &poll);
    }

  return 0;
}
