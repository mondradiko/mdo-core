/** @file gpu_vector.c
 */

#include "gpu/gpu_vector.h"
#include "gpu/gpu_device.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): custom allocation */
#include <stdlib.h> /* for mem alloc */
#include <string.h> /* for memcpy */
#include <vulkan/vulkan_core.h>

struct gpu_vector_s
{
  gpu_device_t *gpu;
  VkDevice vkd;

  VkBuffer buffer;
  VkDeviceMemory memory;
  VkBufferUsageFlags usage;
  size_t size;
};

static int
create_buffer (gpu_vector_t *vec)
{
  uint32_t family_index = gpu_device_gfx_family (vec->gpu);

  VkBufferCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = vec->size,
    .usage = vec->usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices = &family_index,
  };

  if (vkCreateBuffer (vec->vkd, &ci, NULL, &vec->buffer) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create GPU buffer\n");
      return 1;
    }

  return 0;
}

static int
find_memory_type (gpu_device_t *gpu, uint32_t type_filter,
                  VkMemoryPropertyFlags desired)
{
  VkPhysicalDevice vkpd = gpu_device_get_physical (gpu);
  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties (vkpd, &properties);

  for (int i = 0; i < properties.memoryTypeCount; i++)
    {
      if ((type_filter & (1 << i))
          && (properties.memoryTypes[i].propertyFlags & desired) == desired)
        return i;
    }

  fprintf (stderr, "failed to find suitable memory type\n");
  return -1;
}

static int
allocate_memory (gpu_vector_t *vec)
{
  VkMemoryRequirements reqs;
  vkGetBufferMemoryRequirements (vec->vkd, vec->buffer, &reqs);

  VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                       | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  int memory_type_index
      = find_memory_type (vec->gpu, reqs.memoryTypeBits, memory_flags);

  if (memory_type_index < 0)
    {
      fprintf (stderr, "failed to find required memory type\n");
      return 1;
    }

  VkMemoryAllocateInfo ai = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .memoryTypeIndex = memory_type_index,
    .allocationSize = vec->size,
  };

  if (vkAllocateMemory (vec->vkd, &ai, NULL, &vec->memory) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to allocate GPU memory\n");
      return 1;
    }

  if (vkBindBufferMemory (vec->vkd, vec->buffer, vec->memory, 0) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to bind buffer memory\n");
      return 1;
    }

  return 0;
}
int
gpu_vector_new (gpu_vector_t **new_vec, gpu_device_t *gpu,
                VkBufferUsageFlags usage)
{
  gpu_vector_t *vec = malloc (sizeof (gpu_vector_t));
  *new_vec = vec;

  vec->gpu = gpu;
  vec->vkd = gpu_device_get (gpu);

  vec->memory = VK_NULL_HANDLE;
  vec->buffer = VK_NULL_HANDLE;
  vec->usage = usage;
  vec->size = 1024;

  if (create_buffer (vec))
    return 1;

  if (allocate_memory (vec))
    return 1;

  return 0;
}

size_t
gpu_vector_size (gpu_vector_t *vec)
{
  if (!vec->buffer || !vec->memory)
    return 0;

  return vec->size;
}

void
gpu_vector_delete (gpu_vector_t *vec)
{
  if (vec->buffer)
    vkDestroyBuffer (vec->vkd, vec->buffer, NULL);

  if (vec->memory)
    vkFreeMemory (vec->vkd, vec->memory, NULL);

  free (vec);
}

int
gpu_vector_reserve (gpu_vector_t *vec, size_t required_size)
{
  int resize_needed = 0;
  while (required_size > vec->size)
    {
      resize_needed = 1;
      vec->size += vec->size >> 1; /* growth factor of 1.5 */
    }

  if (resize_needed)
    {
      if (vec->buffer)
        vkDestroyBuffer (vec->vkd, vec->buffer, NULL);

      if (vec->memory)
        vkFreeMemory (vec->vkd, vec->memory, NULL);

      if (create_buffer (vec))
        {
          fprintf (stderr, "failed to resize GPU buffer\n");
          return 1;
        }

      if (allocate_memory (vec))
        {
          fprintf (stderr, "failed to resize GPU memory\n");
          return 1;
        }
    }

  return 0;
}

int
gpu_vector_write (gpu_vector_t *vec, const void *src, size_t size, size_t num)
{
  size_t copy_size = size * num;

  if (gpu_vector_reserve (vec, copy_size))
    {
      fprintf (stderr, "failed to reserve GPU memory for transfer\n");
      return 1;
    }

  void *dst = NULL;
  if (vkMapMemory (vec->vkd, vec->memory, 0, copy_size, 0, &dst) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to map GPU memory\n");
      return 1;
    }

  memcpy (dst, src, copy_size);

  vkUnmapMemory (vec->vkd, vec->memory);

  return 0;
}

VkBuffer
gpu_vector_get (gpu_vector_t *vec)
{
  if (!vec->buffer)
    fprintf (stderr, "WARN: retrieving unitialized GPU buffer\n");

  return vec->buffer;
}
