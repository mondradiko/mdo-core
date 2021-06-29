/** @file camera.c
 */

#include "renderer/camera.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): mdo_allocator */
#include <stdlib.h> /* for mem alloc */
#include <vulkan/vulkan_core.h>

struct camera_s
{
  gpu_device_t *gpu;
  VkDevice vkd;
  VkRenderPass rp;
  viewport_t *viewports[MAX_VIEWPORTS_PER_CAMERA];
  int viewport_num;
};

static int
create_render_pass (camera_t *cam)
{
  VkAttachmentDescription swapchain_desc = {
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference swapchain_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription composite_sp = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &swapchain_ref,
  };

  VkSubpassDependency acquire_dep = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  VkRenderPassCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &swapchain_desc,
    .subpassCount = 1,
    .pSubpasses = &composite_sp,
    .dependencyCount = 1,
    .pDependencies = &acquire_dep,
  };

  if (vkCreateRenderPass (cam->vkd, &ci, NULL, &cam->rp) != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create render pass\n");
      return 1;
    }

  return 0;
}

int
camera_new (camera_t **new_cam, const struct camera_config *config)
{
  if (config->viewport_num > MAX_VIEWPORTS_PER_CAMERA)
    {
      fprintf (stderr, "too many viewports\n");
      return 1;
    }

  camera_t *cam = malloc (sizeof (camera_t));
  *new_cam = cam;

  cam->gpu = config->gpu;
  cam->vkd = gpu_device_get (cam->gpu);
  cam->rp = VK_NULL_HANDLE;
  cam->viewport_num = 0;

  if (create_render_pass (cam))
    return 1;

  for (int i = 0; i < config->viewport_num; i++)
    {
      cam->viewport_num++;
      if (viewport_new (&cam->viewports[i], cam->rp,
                        &config->viewport_configs[i]))
        {
          fprintf (stderr, "failed to create viewport\n");
          return 1;
        }
    }

  return 0;
}

void
camera_delete (camera_t *cam)
{
  for (int i = 0; i < cam->viewport_num; i++)
    viewport_delete (cam->viewports[i]);

  if (cam->rp)
    vkDestroyRenderPass (cam->vkd, cam->rp, NULL);

  free (cam);
}

int
camera_acquire (camera_t *cam, viewport_t **viewports)
{
  int viewport_num = cam->viewport_num;

  for (int i = 0; i < viewport_num; i++)
    viewports[i] = cam->viewports[i];

  return viewport_num;
}
