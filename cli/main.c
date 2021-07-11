#include <stdio.h>

#include "displays/display.h"
#include "displays/sdl/sdl_display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "log.h"
#include "renderer/renderer.h"
#include "world/world.h"

typedef struct cli_state_s
{
  sdl_display_t *dp;
  gpu_device_t *gpu;
  renderer_t *ren;
  world_t *w;
} cli_state_t;

int
init_cli_state (cli_state_t *cli)
{
  cli->dp = NULL;
  cli->gpu = NULL;
  cli->ren = NULL;
  cli->w = NULL;

  if (sdl_display_new (&cli->dp))
    {
      LOG_ERR ("failed to create SDL display");
      return 1;
    }

  struct vk_config_t vk_config;
  sdl_display_vk_config (cli->dp, &vk_config);

  if (gpu_device_new (&cli->gpu, &vk_config))
    {
      LOG_ERR ("failed to create GPU device");
      return 1;
    }

  if (sdl_display_begin_session (cli->dp, cli->gpu))
    {
      LOG_ERR ("failed to begin SDL session");
      return 1;
    }

  VkRenderPass rp = camera_get_render_pass (sdl_display_get_camera (cli->dp));
  if (renderer_new (&cli->ren, cli->gpu, rp))
    {
      LOG_ERR ("failed to create renderer");
      return 1;
    }

  if (world_new (&cli->w, renderer_get_debug_draw_list (cli->ren)))
    {
      LOG_ERR ("failed to create world");
      return 1;
    }

  return 0;
}

void
cleanup_cli_state (cli_state_t *cli)
{
  if (cli->w)
    world_delete (cli->w);

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

static void
temporary_debug_draw (debug_draw_list_t *ddl)
{

  debug_draw_vertex_t vertex1 = {
    .color = { 1.0, 0.0, 0.0 },
    .position = { 0.5, 0.5, 0.0 },
  };

  debug_draw_vertex_t vertex2 = {
    .color = { 0.0, 1.0, 0.0 },
    .position = { 0.5, -0.5, 0.0 },
  };

  debug_draw_index_t index1 = debug_draw_list_vertex (ddl, &vertex1);
  debug_draw_index_t index2 = debug_draw_list_vertex (ddl, &vertex2);

  debug_draw_list_line (ddl, index1, index2);
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

      if (poll.should_run)
        {
          world_step (cli.w, poll.dt);
        }

      if (poll.should_render)
        {
          camera_t *camera = sdl_display_camera (cli.dp);
          temporary_debug_draw (renderer_get_debug_draw_list (cli.ren));
          renderer_render_frame (cli.ren, &camera, 1);
        }
    }

  cleanup_cli_state (&cli);
  LOG_MSG ("Mondradiko exited successfully.\nHave a nice day! :)");
  return 0;
}
