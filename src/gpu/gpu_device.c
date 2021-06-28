/** @file gpu_device.c
 */

#include "gpu/gpu_device.h"

/* TODO(marceline-cramer): use mdo-allocator */
#include <stdlib.h> /* for mem alloc */
/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct gpu_device_s
{
  VkInstance instance;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                void *userdata)
{
  fprintf (stderr, "vulkan validation: %s\n", callback_data->pMessage);
  return VK_FALSE;
}

int
gpu_device_new (gpu_device_t **new_gpu)
{
  gpu_device_t *gpu = malloc (sizeof (gpu_device_t));
  *new_gpu = gpu;

  gpu->instance = VK_NULL_HANDLE;

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,

    /* TODO(marceline-cramer): read these parameters from requirements */
    .pApplicationName = "Mondradiko",
    .applicationVersion = VK_MAKE_VERSION (0, 0, 0),
    .pEngineName = "Mondradiko",
    .engineVersion = VK_MAKE_VERSION (0, 0, 0),

    .apiVersion = VK_API_VERSION_1_0,
  };

  const char *layers[] = { "VK_LAYER_KHRONOS_validation" };

  VkInstanceCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = 1,
    .ppEnabledLayerNames = layers,
    .enabledExtensionCount = 0,
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
      return 1;
    }

  return 0;
}

void
gpu_device_delete (gpu_device_t *gpu)
{
  if (gpu->instance)
    vkDestroyInstance (gpu->instance, NULL);

  free (gpu);
}
