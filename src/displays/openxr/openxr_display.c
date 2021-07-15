/** @file openxr_display.c
 */

#include "displays/openxr/openxr_display.h"

#include "log.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

#include <openxr/openxr.h>

struct openxr_display_s
{
  XrInstance instance;
};

static int
create_instance (openxr_display_t *dp)
{
  XrInstanceCreateInfo ci = {
    .type = XR_TYPE_INSTANCE_CREATE_INFO,
    .enabledApiLayerCount = 0,
    .enabledExtensionCount = 0,
  };

  ci.applicationInfo = (XrApplicationInfo){};

  if (xrCreateInstance (&ci, &dp->instance) != XR_SUCCESS)
    {
      LOG_ERR ("failed to create instance");
      return 1;
    }

  return 0;
}

int
openxr_display_new (openxr_display_t **new_dp)
{
  openxr_display_t *dp = malloc (sizeof (openxr_display_t));
  *new_dp = dp;

  dp->instance = XR_NULL_HANDLE;

  if (create_instance (dp))
    return 1;

  LOG_MSG ("OpenXR actually succeeded... but my GPU is broken");
  LOG_MSG ("if I forget to remove these log messages then yell at me");
  LOG_MSG (" - Marceline");
  return 1;
}

void
openxr_display_delete (openxr_display_t *dp)
{
  if (dp->instance)
    xrDestroyInstance (dp->instance);

  free (dp);
}

void
openxr_display_vk_config (openxr_display_t *dp, struct vk_config_t *v)
{
}

camera_t *
openxr_display_get_camera (openxr_display_t *dp)
{
  return NULL;
}

int
openxr_display_begin_session (openxr_display_t *dp, gpu_device_t *gpu)
{
  return 0;
}

void
openxr_display_end_session (openxr_display_t *dp)
{
}

void
openxr_display_poll (openxr_display_t *dp, struct display_poll_t *poll)
{
  poll->should_exit = 0;
  poll->should_render = 1;
  poll->should_run = 1;
}
