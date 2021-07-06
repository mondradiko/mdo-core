/** @file sdl_display.c
 */

#include "displays/sdl/sdl_display.h"

#include "displays/display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "renderer/camera.h"
#include "renderer/viewport.h"

/* TODO(marceline-cramer): replace with mdo_allocator */
#include <stdio.h>  /* for fprintf */
#include <stdlib.h> /* for mem alloc*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAX_VULKAN_EXTENSIONS 16

struct sdl_display_s
{
  SDL_Window *window;
  char *instance_extensions;

  /* session data */
  gpu_device_t *gpu;
  VkSurfaceKHR surface;
  camera_t *camera;
};

static int
create_window (sdl_display_t *dp)
{
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

static int
get_instance_extensions (sdl_display_t *dp)
{
  unsigned int num_exts = MAX_VULKAN_EXTENSIONS;
  const char *ext_list[MAX_VULKAN_EXTENSIONS];
  SDL_Vulkan_GetInstanceExtensions (dp->window, &num_exts, ext_list);

  int list_len = num_exts; /* room for spaces and null terminator */
  for (int i = 0; i < num_exts; i++)
    {
      list_len += strlen (ext_list[i]);
    }

  dp->instance_extensions = malloc (list_len);
  char *ptr = dp->instance_extensions;
  for (int i = 0; i < num_exts; i++)
    {
      size_t element_len = strlen (ext_list[i]);
      memcpy (ptr, ext_list[i], element_len);
      ptr += element_len;

      if (i < num_exts - 1)
        *ptr++ = ' '; /* add space */
    }

  *ptr = '\0'; /* add null terminator */

  return 0;
}

int
sdl_display_new (sdl_display_t **new_dp)
{
  if (SDL_Init (SDL_INIT_VIDEO))
    {
      fprintf (stderr, "failed to init SDL\n");
      return -1;
    }

  sdl_display_t *dp = malloc (sizeof (sdl_display_t));
  *new_dp = dp;

  dp->instance_extensions = NULL;
  dp->surface = VK_NULL_HANDLE;
  dp->camera = NULL;

  if (create_window (dp))
    return -1;

  if (get_instance_extensions (dp))
    return -1;

  return 0;
}

void
sdl_display_delete (sdl_display_t *dp)
{
  sdl_display_end_session (dp);

  if (dp->window)
    SDL_DestroyWindow (dp->window);

  if (dp->instance_extensions)
    free (dp->instance_extensions);

  free (dp);

  SDL_Quit ();
}

void
sdl_display_vk_config (sdl_display_t *dp, struct vk_config_t *config)
{
  config->min_api_version = VK_API_VERSION_1_0;
  config->max_api_version = VK_API_VERSION_1_2;
  config->instance_extensions = dp->instance_extensions;
  config->device_extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  config->physical_device = VK_NULL_HANDLE;
}

int
sdl_display_begin_session (sdl_display_t *dp, gpu_device_t *gpu)
{
  dp->gpu = gpu;

  SDL_Window *window = dp->window;
  VkInstance instance = gpu_device_get_instance (gpu);
  VkSurfaceKHR *surface = &dp->surface;
  if (SDL_Vulkan_CreateSurface (window, instance, surface) != SDL_TRUE)
    {
      fprintf (stderr, "failed to create window surface: %s\n",
               SDL_GetError ());
      return 1;
    }

  int width;
  int height;
  SDL_Vulkan_GetDrawableSize (dp->window, &width, &height);

  struct viewport_config vp_config = {
    .gpu = dp->gpu,
    .type = VIEWPORT_TYPE_SURFACE,
    .width = width,
    .height = height,

    .sub = {
      .surface = dp->surface,
    },
  };

  struct camera_config cam_config = {
    .gpu = dp->gpu,
    .viewport_configs = &vp_config,
    .viewport_num = 1,
  };

  if (camera_new (&dp->camera, &cam_config))
    {
      fprintf (stderr, "failed to create camera\n");
      return 1;
    }

  return 0;
}

void
sdl_display_end_session (sdl_display_t *dp)
{
  if (dp->camera)
    camera_delete (dp->camera);

  if (dp->surface)
    vkDestroySurfaceKHR (gpu_device_get_instance (dp->gpu), dp->surface, NULL);

  dp->camera = NULL;
  dp->surface = VK_NULL_HANDLE;
}

camera_t *
sdl_display_get_camera (sdl_display_t *dp)
{
  return dp->camera;
}

void
sdl_display_poll (sdl_display_t *dp, struct display_poll_t *poll)
{
  poll->should_exit = 0;
  poll->should_run = 1;
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

camera_t *
sdl_display_camera (sdl_display_t *dp)
{
  return dp->camera;
}
