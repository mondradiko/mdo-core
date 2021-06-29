/** @file gpu_device.c
 */

#include "gpu/gpu_device.h"

#include "gpu/vk_config.h"

/* TODO(marceline-cramer): use mdo-allocator */
#include <stdlib.h> /* for mem alloc */
/* TODO(marceline-cramer): custom logging */
#include <stdio.h>  /* for fprintf */
#include <string.h> /* for strlen, memcpy */

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAX_EXTENSIONS 32

struct gpu_device_s
{
  VkInstance instance;
  VkPhysicalDevice physical_device;
  uint32_t gfx_queue_family;
  VkDevice device;
};

static int
split_list (char *list, const char *array[MAX_EXTENSIONS])
{
  size_t len = strlen (list);
  if (len == 0)
    return 0;

  int num_elements = 0;
  for (int i = 0; i <= len; i++)
    {
      switch (list[i])
        {
        case ' ':
          list[i] = '\n';
        case '\0':
          num_elements++;
        default:
          break;
        }
    }

  fprintf (stdout, "num elements: %d\n%s\n", num_elements, list);

  if (num_elements > MAX_EXTENSIONS)
    {
      fprintf (stderr, "ERROR: number of elements in list exceeds maximum\n");
      return 0;
    }

  int idx = 0;
  const char *last_element = list;
  for (int i = 0; i <= len; i++)
    {
      if (list[i] == '\n' || list[i] == '\0')
        {
          list[i] = '\0';
          array[idx] = last_element;
          last_element = &list[i + 1];
          idx++;
        }
    }

  return num_elements;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                void *userdata)
{
  fprintf (stderr, "vulkan validation: %s\n", callback_data->pMessage);
  return VK_FALSE;
}

static int
create_instance (gpu_device_t *gpu, const struct vk_config_t *config)
{
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,

    /* TODO(marceline-cramer): read these parameters from requirements */
    .pApplicationName = "Mondradiko",
    .applicationVersion = VK_MAKE_VERSION (0, 0, 0),
    .pEngineName = "Mondradiko",
    .engineVersion = VK_MAKE_VERSION (0, 0, 0),

    .apiVersion = config->min_api_version,
  };

  char *instance_ext_list = malloc (strlen (config->instance_extensions) + 1);
  strcpy (instance_ext_list, config->instance_extensions);
  const char *instance_exts[MAX_EXTENSIONS];
  int instance_ext_num = split_list (instance_ext_list, instance_exts);

  const char *layers[] = { "VK_LAYER_KHRONOS_validation" };

  VkInstanceCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = 1,
    .ppEnabledLayerNames = layers,
    .enabledExtensionCount = instance_ext_num,
    .ppEnabledExtensionNames = instance_exts,
  };

  VkDebugUtilsMessengerCreateInfoEXT messenger_ci = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                       | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                       | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debug_callback,
  };

  ci.pNext = &messenger_ci;

  if (vkCreateInstance (&ci, NULL, &gpu->instance) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create Vulkan instance\n");
      free (instance_ext_list);
      return -1;
    }

  free (instance_ext_list);

  return 0;
}

static VkPhysicalDevice
autoselect_physical_device (gpu_device_t *gpu)
{
  uint32_t device_count = 32; /* the user will never have more than 32 GPUs */
  VkPhysicalDevice devices[device_count];

  /* TODO(marceline-cramer) check this result in the case that the user DOES
   * have way too many GPUs */
  vkEnumeratePhysicalDevices (gpu->instance, &device_count, devices);

  /* TODO(marceline-cramer) check for present queue, surface support */
  if (device_count > 0)
    return devices[0];

  return VK_NULL_HANDLE;
}

static int
find_queue_families (gpu_device_t *gpu)
{
  uint32_t num = 32; /* ultra-conservative upper cap */
  VkQueueFamilyProperties props[num];
  vkGetPhysicalDeviceQueueFamilyProperties (gpu->physical_device, &num, props);

  for (int i = 0; i < num; i++)
    {
      if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          gpu->gfx_queue_family = i;
          return 0;
        }
    }

  fprintf (stderr, "failed to find necessary queue families\n");
  return -1;
}

static int
create_logical_device (gpu_device_t *gpu, const struct vk_config_t *config)
{
  char *device_ext_list = malloc (strlen (config->device_extensions) + 1);
  strcpy (device_ext_list, config->device_extensions);
  const char *device_exts[MAX_EXTENSIONS];
  int device_ext_num = split_list (device_ext_list, device_exts);

  float queue_priority = 1.0f;

  VkDeviceQueueCreateInfo queue_ci = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = gpu->gfx_queue_family,
    .queueCount = 1,
    .pQueuePriorities = &queue_priority,
  };

  VkPhysicalDeviceFeatures device_features = {};

  VkDeviceCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queue_ci,
    .enabledExtensionCount = device_ext_num,
    .ppEnabledExtensionNames = device_exts,
    .pEnabledFeatures = &device_features,
  };

  if (vkCreateDevice (gpu->physical_device, &ci, NULL, &gpu->device)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create Vulkan logical device");
      free (device_ext_list);
      return -1;
    }

  free (device_ext_list);
  return 0;
}

int
gpu_device_new (gpu_device_t **new_gpu, const struct vk_config_t *config)
{
  gpu_device_t *gpu = malloc (sizeof (gpu_device_t));
  *new_gpu = gpu;

  gpu->instance = VK_NULL_HANDLE;
  gpu->physical_device = VK_NULL_HANDLE;
  gpu->device = VK_NULL_HANDLE;

  if (create_instance (gpu, config))
    return -1;

  if (config->physical_device != VK_NULL_HANDLE)
    gpu->physical_device = config->physical_device;
  else
    gpu->physical_device = autoselect_physical_device (gpu);

  if (gpu->physical_device == VK_NULL_HANDLE)
    {
      fprintf (stderr, "failed to find Vulkan physical device\n");
      return -1;
    }

  if (find_queue_families (gpu))
    return -1;

  if (create_logical_device (gpu, config))
    return -1;

  return 0;
}

void
gpu_device_delete (gpu_device_t *gpu)
{
  if (gpu->device)
    vkDestroyDevice (gpu->device, NULL);

  if (gpu->instance)
    vkDestroyInstance (gpu->instance, NULL);

  free (gpu);
}

VkInstance
gpu_device_get_instance (gpu_device_t *gpu)
{
  if (!gpu->instance)
    fprintf (stderr, "WARN: getting null Vulkan instance\n");

  return gpu->instance;
}

VkPhysicalDevice
gpu_device_get_physical (gpu_device_t *gpu)
{
  if (!gpu->physical_device)
    fprintf (stderr, "WARN: getting null Vulkan physical device\n");

  return gpu->physical_device;
}

VkDevice
gpu_device_get (gpu_device_t *gpu)
{
  if (!gpu->device)
    fprintf (stderr, "WARN: getting null Vulkan device\n");

  return gpu->device;
}

int
gpu_device_gfx_family (gpu_device_t *gpu)
{
  return gpu->gfx_queue_family;
}
