#include <stdio.h>

#include "displays/display.h"
#include "displays/sdl/sdl_display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "renderer/renderer.h"

typedef struct cli_state_s
{
  sdl_display_t *dp;
  gpu_device_t *gpu;
  renderer_t *ren;
} cli_state_t;

int
init_cli_state (cli_state_t *cli)
{
  cli->dp = NULL;
  cli->gpu = NULL;

  if (sdl_display_new (&cli->dp))
    {
      fprintf (stderr, "failed to create SDL display\n");
      return 1;
    }

  struct vk_config_t vk_config;
  sdl_display_vk_config (cli->dp, &vk_config);

  if (gpu_device_new (&cli->gpu, &vk_config))
    {
      fprintf (stderr, "failed to create GPU device\n");
      return 1;
    }

  if (sdl_display_begin_session (cli->dp, cli->gpu))
    {
      fprintf (stderr, "failed to begin SDL session\n");
      return 1;
    }

  if (renderer_new (&cli->ren, cli->gpu))
    {
      fprintf (stderr, "failed to create renderer\n");
      return 1;
    }

  return 0;
}

void
cleanup_cli_state (cli_state_t *cli)
{
  if (cli->ren)
    renderer_delete (cli->ren);

  if (cli->dp)
    {
      sdl_display_end_session (cli->dp);
      sdl_display_delete (cli->dp);
    }

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

      if (poll.should_render)
        renderer_render_frame (cli.ren);
    }

  return 0;
}
