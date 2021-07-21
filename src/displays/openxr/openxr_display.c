/** @file openxr_display.c
 */

#include "displays/openxr/openxr_display.h"

#include "gpu/gpu_device.h"
#include "gpu/vk_config.h"
#include "log.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define DECLARE_XR_FN_PTR(name) PFN_##name ext_##name;
#define LOAD_XR_FN_PTR(name)                                                  \
  xrGetInstanceProcAddr (dp->instance, #name,                                 \
                         (PFN_xrVoidFunction *)&dp->ext_##name)

struct openxr_display_s
{
  XrInstance instance;
  XrSystemId system_id;

  /* loaded instance functions */
  DECLARE_XR_FN_PTR (xrGetVulkanGraphicsRequirementsKHR);
  DECLARE_XR_FN_PTR (xrGetVulkanInstanceExtensionsKHR);
  DECLARE_XR_FN_PTR (xrGetVulkanGraphicsDeviceKHR);
  DECLARE_XR_FN_PTR (xrGetVulkanDeviceExtensionsKHR);

  /* Vulkan configuration data */
  XrGraphicsRequirementsVulkanKHR graphics_reqs;
  VkPhysicalDevice vkpd;
  char *vk_instance_exts;
  char *vk_device_exts;

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

static void
load_fn_ptrs (openxr_display_t *dp)
{
  LOAD_XR_FN_PTR (xrGetVulkanGraphicsRequirementsKHR);
  LOAD_XR_FN_PTR (xrGetVulkanInstanceExtensionsKHR);
  LOAD_XR_FN_PTR (xrGetVulkanGraphicsDeviceKHR);
  LOAD_XR_FN_PTR (xrGetVulkanDeviceExtensionsKHR);
}

static int
load_vk_config (openxr_display_t *dp)
{
  XrInstance xri = dp->instance;
  XrSystemId sid = dp->system_id;

  dp->graphics_reqs = (XrGraphicsRequirementsVulkanKHR){
    .type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR,
  };

  if (dp->ext_xrGetVulkanGraphicsRequirementsKHR (xri, sid, &dp->graphics_reqs)
      != XR_SUCCESS)
    {
      LOG_ERR ("failed to get Vulkan graphics requirements");
      return 1;
    }

  uint32_t vki_ext_len;
  uint32_t vkd_ext_len;

  dp->ext_xrGetVulkanInstanceExtensionsKHR (xri, sid, 0, &vki_ext_len, NULL);
  dp->ext_xrGetVulkanDeviceExtensionsKHR (xri, sid, 0, &vkd_ext_len, NULL);

  dp->vk_instance_exts = malloc (vki_ext_len);
  dp->vk_device_exts = malloc (vkd_ext_len);

  dp->ext_xrGetVulkanInstanceExtensionsKHR (
      xri, sid, vki_ext_len, &vki_ext_len, dp->vk_instance_exts);
  dp->ext_xrGetVulkanDeviceExtensionsKHR (xri, sid, vkd_ext_len, &vkd_ext_len,
                                          dp->vk_device_exts);

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

  dp->vk_instance_exts = NULL;
  dp->vk_device_exts = NULL;

  if (create_instance (dp))
    return 1;

  if (find_system (dp))
    return 1;

  load_fn_ptrs (dp);

  if (load_vk_config (dp))
    return 1;

  return 0;
}

void
openxr_display_delete (openxr_display_t *dp)
{
  openxr_display_end_session (dp);

  if (dp->instance)
    xrDestroyInstance (dp->instance);

  if (dp->vk_instance_exts)
    free (dp->vk_instance_exts);

  if (dp->vk_device_exts)
    free (dp->vk_device_exts);

  free (dp);
}

static int
select_physical_device (void *data, VkInstance vki, VkPhysicalDevice *vkpd)
{
  openxr_display_t *dp = data;
  XrInstance xri = dp->instance;
  XrSystemId sid = dp->system_id;
  if (dp->ext_xrGetVulkanGraphicsDeviceKHR (xri, sid, vki, vkpd) != XR_SUCCESS)
    {
      LOG_ERR ("failed to get Vulkan graphics device");
      return 1;
    }

  return 0;
}

void
openxr_display_vk_config (openxr_display_t *dp, vk_config_t *config)
{
  config->min_api_version = dp->graphics_reqs.minApiVersionSupported;
  config->max_api_version = dp->graphics_reqs.maxApiVersionSupported;
  config->instance_extensions = dp->vk_instance_exts;
  config->device_extensions = dp->vk_device_exts;
  config->select_physical_device_cb = select_physical_device;
  config->data = dp;
}

camera_t *
openxr_display_get_camera (openxr_display_t *dp)
{
  return NULL;
}

static int
create_stage_space (openxr_display_t *dp)
{
  XrPosef pose = { 0 };
  pose.orientation = (XrQuaternionf){ 0.0, 0.0, 0.0, 1.0 };
  pose.position = (XrVector3f){ 0.0, 0.0, 0.0 };
  XrReferenceSpaceCreateInfo ci = {
    .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
    .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE,
    .poseInReferenceSpace = pose,
  };

  if (xrCreateReferenceSpace (dp->session, &ci, &dp->stage_space)
      != XR_SUCCESS)
    {
      LOG_ERR ("failed to create stage space");
      return 1;
    }

  return 0;
}

int
openxr_display_begin_session (openxr_display_t *dp, gpu_device_t *gpu)
{
  XrGraphicsBindingVulkanKHR gb_vk = {
    .type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
    .instance = gpu_device_get_instance (gpu),
    .physicalDevice = gpu_device_get_physical (gpu),
    .device = gpu_device_get (gpu),
    .queueFamilyIndex = gpu_device_gfx_family (gpu),
  };

  XrSessionCreateInfo ci = {
    .type = XR_TYPE_SESSION_CREATE_INFO,
    .next = &gb_vk,
    .systemId = dp->system_id,
  };

  if (xrCreateSession (dp->instance, &ci, &dp->session) != XR_SUCCESS)
    {
      LOG_ERR ("failed to create session");
      return 1;
    }

  if (create_stage_space (dp))
    return 1;

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
