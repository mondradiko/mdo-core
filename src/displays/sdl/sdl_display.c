/** @file sdl_display.c
 */

#include "displays/sdl/sdl_display.h"

#include "displays/display.h"
#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"

/* TODO(marceline-cramer): replace with mdo_allocator */
#include <stdio.h>  /* for fprintf */
#include <stdlib.h> /* for mem alloc*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

struct sdl_display_s
{
  SDL_Window *window;
  char *instance_extensions;

  /* session data */
  gpu_device_t *gpu;
  VkSurfaceKHR surface;
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
  unsigned int num_exts = 16;
  const char *ext_list[num_exts];
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
  sdl_display_t *dp = malloc (sizeof (sdl_display_t));
  *new_dp = dp;

  dp->instance_extensions = NULL;
  dp->surface = VK_NULL_HANDLE;

  if (create_window (dp))
    return -1;

  if (get_instance_extensions (dp))
    return -1;

  return 0;
}

void
sdl_display_delete (sdl_display_t *dp)
{
  if (dp->window)
    SDL_DestroyWindow (dp->window);

  if (dp->instance_extensions)
    free (dp->instance_extensions);

  free (dp);
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

  return 0;
}

void
sdl_display_end_session (sdl_display_t *dp)
{
  if (dp->surface)
    vkDestroySurfaceKHR (gpu_device_get_instance (dp->gpu), dp->surface, NULL);

  dp->surface = VK_NULL_HANDLE;
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
