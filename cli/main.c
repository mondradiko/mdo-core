#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "displays/display.h"
#include "displays/sdl/sdl_display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "log.h"
#include "network/network_client.h"
#include "network/network_server.h"
#include "renderer/renderer.h"
#include "world/world.h"

typedef struct cli_state_s
{
  /* params */
  int is_headless;
  int is_client;

  /* objects */
  sdl_display_t *dp;
  gpu_device_t *gpu;
  renderer_t *ren;
  world_t *w;

  union
  {
    network_client_t *client;
    network_server_t *server;
  } network;
} cli_state_t;

void
print_help (const char *argv0)
{
  fprintf (stderr, "Usage\n  %s [--headless] [--server]", argv0);
}

int
parse_cli_args (cli_state_t *cli, int argc, const char *argv[])
{
  cli->is_headless = 0;
  cli->is_client = 1;

  for (int i = 1; i < argc; i++)
    {
      const char *arg = argv[i];

      if (strcmp (arg, "--headless") == 0)
        {
          cli->is_headless = 1;
        }
      else if (strcmp (arg, "--server") == 0)
        {
          cli->is_client = 0;
        }
      else
        {
          print_help (argv[0]);
          return 1;
        }
    }

  return 0;
}

void
init_cli_state (cli_state_t *cli)
{
  cli->dp = NULL;
  cli->gpu = NULL;
  cli->ren = NULL;
  cli->w = NULL;

  if (cli->is_client)
    cli->network.client = NULL;
  else
    cli->network.server = NULL;
}

int
create_cli_objects (cli_state_t *cli)
{

  if (!cli->is_headless)
    {
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

      VkRenderPass rp
          = camera_get_render_pass (sdl_display_get_camera (cli->dp));
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
    }

  if (cli->is_client)
    {
      if (network_client_new (&cli->network.client))
        {
          LOG_ERR ("failed to create network client");
          return 1;
        }

      if (network_client_connect (cli->network.client, "127.0.0.1", 10555))
        {
          LOG_ERR ("failed to connect to server");
          return 1;
        }
    }
  else
    {
      if (network_server_new (&cli->network.server))
        {
          LOG_ERR ("failed to create network server");
          return 1;
        }
    }

  return 0;
}

void
cleanup_cli_state (cli_state_t *cli)
{
  if (cli->is_client)
    {
      if (cli->network.client)
        network_client_delete (cli->network.client);
    }
  else
    {
      if (cli->network.server)
        network_server_delete (cli->network.server);
    }

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

static int g_interrupted = 0;

void
signal_handler (int signum)
{
  LOG_MSG ("interrupt signal %d received", signum);
  g_interrupted = 1;
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
main (int argc, const char *argv[])
{
  cli_state_t cli;

  int result = parse_cli_args (&cli, argc, argv);
  if (result)
    return result;

  init_cli_state (&cli);
  result = create_cli_objects (&cli);
  if (result)
    return result;

  if (signal (SIGTERM, signal_handler) == SIG_ERR)
    LOG_WRN ("can't catch SIGTERM");

  if (signal (SIGINT, signal_handler) == SIG_ERR)
    LOG_WRN ("can't catch SIGINT");

  struct display_poll_t poll;
  poll.should_exit = 0;
  while (!poll.should_exit && !g_interrupted)
    {
      if (!cli.is_headless)
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

      if (cli.is_client)
        {
          network_client_update (cli.network.client);
        }
      else
        {
          network_server_update (cli.network.server);
        }
    }

  cleanup_cli_state (&cli);
  LOG_MSG ("Mondradiko exited successfully.\nHave a nice day! :)");
  return 0;
}
