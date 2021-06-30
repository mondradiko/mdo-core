/** @file gpu_shader.c
 */

#include "gpu/gpu_shader.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf, file I/O */
/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */
#include <vulkan/vulkan_core.h>

struct gpu_shader_s
{
  gpu_device_t *gpu;
  VkShaderStageFlags stage;

  VkShaderModule module;
};

int
gpu_shader_new (gpu_shader_t **new_shader, gpu_device_t *gpu,
                VkShaderStageFlags stage)
{
  gpu_shader_t *shader = malloc (sizeof (gpu_shader_t));
  *new_shader = shader;

  shader->gpu = gpu;
  shader->stage = stage;
  shader->module = VK_NULL_HANDLE;

  return 0;
}

void
gpu_shader_delete (gpu_shader_t *shader)
{
  if (shader->module)
    vkDestroyShaderModule (gpu_device_get (shader->gpu), shader->module, NULL);

  free (shader);
}

int
gpu_shader_load_from_file (gpu_shader_t *shader, const char *filename)
{
  FILE *f = fopen (filename, "rb");

  if (!f)
    {
      fprintf (stderr, "failed to open shader file: %s\n", filename);
      return 1;
    }

  fseek (f, 0, SEEK_END);
  size_t code_size = ftell (f);
  fseek (f, 0, SEEK_SET);

  uint32_t *code = malloc (code_size);
  fread (code, code_size, code_size, f);

  VkShaderModuleCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pCode = code,
    .codeSize = code_size,
  };

  VkDevice vkd = gpu_device_get (shader->gpu);
  if (vkCreateShaderModule (vkd, &ci, NULL, &shader->module) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create shader module (file %s)\n", filename);
      free (code);
      fclose (f);
      return 1;
    }

  free (code);
  fclose (f);
  return 0;
}

int
gpu_shader_get (gpu_shader_t *shader, VkPipelineShaderStageCreateInfo *ci)
{
  if (!shader->module)
    {
      fprintf (stderr, "attempted to get unloaded shader\n");
      return 1;
    }

  *ci = (VkPipelineShaderStageCreateInfo){
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .module = shader->module,
    .stage = shader->stage,
    .pName = "main",
  };

  return 0;
}
