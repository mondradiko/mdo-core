/** @file openxr_display.c
 */

#include "displays/openxr/openxr_display.h"

#include "log.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

struct openxr_display_s
{
  XrInstance instance;
  XrSystemId system_id;

  /* session info */
  gpu_device_t *gpu;
  XrSession session;
  XrSpace stage_space;
};

static int
create_instance (openxr_display_t *dp)
{
  const char *extensions[] = {
    XR_KHR_VULKAN_ENABLE_EXTENSION_NAME,
    XR_EXT_DEBUG_UTILS_EXTENSION_NAME,
  };

  XrInstanceCreateInfo ci = {
    .type = XR_TYPE_INSTANCE_CREATE_INFO,
    .enabledApiLayerCount = 0,
    .enabledExtensionCount = 2,
    .enabledExtensionNames = extensions,
  };

  ci.applicationInfo = (XrApplicationInfo){
    /* TODO(marceline-cramer) replace with CMake-generated version */
    .applicationName = "mdo-cli",
    .applicationVersion = XR_MAKE_VERSION (0, 0, 0),
    .engineName = "Mondradiko",
    .engineVersion = XR_MAKE_VERSION (0, 0, 0),
    .apiVersion = XR_MAKE_VERSION (1, 0, 0),
  };

  if (xrCreateInstance (&ci, &dp->instance) != XR_SUCCESS)
    {
      LOG_ERR ("failed to create instance");
      return 1;
    }

  return 0;
}

static int
find_system (openxr_display_t *dp)
{
  XrSystemGetInfo gi = {
    .type = XR_TYPE_SYSTEM_GET_INFO,
    .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
  };

  if (xrGetSystem (dp->instance, &gi, &dp->system_id) != XR_SUCCESS)
    {
      LOG_ERR ("failed to find HMD");
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
  dp->session = XR_NULL_HANDLE;
  dp->stage_space = XR_NULL_HANDLE;

  if (create_instance (dp))
    return 1;

  if (find_system (dp))
    return 1;

  return 0;
}

void
openxr_display_delete (openxr_display_t *dp)
{
  openxr_display_end_session (dp);

  if (dp->instance)
    xrDestroyInstance (dp->instance);

  free (dp);
}

void
openxr_display_vk_config (openxr_display_t *dp, struct vk_config_t *config)
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
  if (dp->stage_space)
    xrDestroySpace (dp->stage_space);

  if (dp->session)
    xrDestroySession (dp->session);

  dp->session = XR_NULL_HANDLE;
  dp->stage_space = XR_NULL_HANDLE;
}

void
openxr_display_poll (openxr_display_t *dp, struct display_poll_t *poll)
{
  poll->should_exit = 0;
  poll->should_render = 1;
  poll->should_run = 1;
}
