/** @file debug_pass.c
 */

#include "renderer/debug/debug_pass.h"

#include "gpu/gpu_device.h"
#include "gpu/gpu_shader.h"
#include "renderer/debug/debug_draw.h"

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */
#include <vulkan/vulkan_core.h>

struct debug_pass_s
{
  renderer_t *ren;
  gpu_device_t *gpu;
  VkDevice vkd;

  debug_draw_list_t *ddl;

  gpu_shader_t *vertex_shader;
  gpu_shader_t *fragment_shader;

  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
};

static int
create_pipeline_layout (debug_pass_t *dbp)
{
  VkPipelineLayoutCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .flags = 0,
    .setLayoutCount = 0,
  };

  if (vkCreatePipelineLayout (dbp->vkd, &ci, NULL, &dbp->pipeline_layout)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create debug pipeline layout\n");
      return 1;
    }

  return 0;
}

static int
load_shaders (debug_pass_t *dbp)
{
  if (gpu_shader_new (&dbp->vertex_shader, dbp->gpu,
                      VK_SHADER_STAGE_VERTEX_BIT))
    {
      fprintf (stderr, "failed to create vertex shader\n");
      return 1;
    }

  if (gpu_shader_new (&dbp->fragment_shader, dbp->gpu,
                      VK_SHADER_STAGE_FRAGMENT_BIT))
    {
      fprintf (stderr, "failed to create fragment shader\n");
      return 1;
    }

  static const char *VERTEX_SOURCE = "./shaders/debug.vert.spv";
  static const char *FRAGMENT_SOURCE = "./shaders/debug.frag.spv";

  if (gpu_shader_load_from_file (dbp->vertex_shader, VERTEX_SOURCE))
    return 1;

  if (gpu_shader_load_from_file (dbp->fragment_shader, FRAGMENT_SOURCE))
    return 1;

  return 0;
}

static int
create_pipeline (debug_pass_t *dbp, VkRenderPass rp)
{
  VkPipelineShaderStageCreateInfo shader_stages[2];
  gpu_shader_get (dbp->vertex_shader, &shader_stages[0]);
  gpu_shader_get (dbp->fragment_shader, &shader_stages[1]);

  VkVertexInputBindingDescription binding_desc = {
    .binding = 0,
    .stride = sizeof (debug_draw_vertex_t),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  VkVertexInputAttributeDescription attribute_descs[2];

  attribute_descs[0] = (VkVertexInputAttributeDescription){
    .binding = 0,
    .location = 0,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = offsetof (debug_draw_vertex_t, position),
  };

  attribute_descs[1] = (VkVertexInputAttributeDescription){
    .binding = 0,
    .location = 1,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = offsetof (debug_draw_vertex_t, color),
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_desc,
    .vertexAttributeDescriptionCount = 2,
    .pVertexAttributeDescriptions = attribute_descs,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
  };

  VkViewport viewport = {0};
  VkRect2D scissor = {0};

  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0,
  };

  VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                      | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE,
  };

  VkPipelineColorBlendStateCreateInfo color_blend_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment,
  };

  VkDynamicState dynamic_states[]
      = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamic_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamic_states,
  };

  VkGraphicsPipelineCreateInfo ci = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState = &multisample_state,
    .pDepthStencilState = &depth_stencil_state,
    .pColorBlendState = &color_blend_state,
    .pDynamicState = &dynamic_state,
    .layout = dbp->pipeline_layout,
    .renderPass = rp,
    .subpass = 0,
  };

  VkPipelineCache cache = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines (dbp->vkd, cache, 1, &ci, NULL, &dbp->pipeline)
      != VK_SUCCESS)
    {
      fprintf (stderr, "failed to create debug pipeline\n");
      return 1;
    }

  return 0;
}

int
debug_pass_new (debug_pass_t **new_dbp, renderer_t *ren, VkRenderPass rp)
{
  debug_pass_t *dbp = malloc (sizeof (debug_pass_t));
  *new_dbp = dbp;

  dbp->ren = ren;
  dbp->gpu = renderer_get_gpu (ren);
  dbp->vkd = gpu_device_get (dbp->gpu);

  dbp->ddl = NULL;

  dbp->vertex_shader = NULL;
  dbp->fragment_shader = NULL;

  dbp->pipeline_layout = VK_NULL_HANDLE;
  dbp->pipeline = VK_NULL_HANDLE;

  if (debug_draw_list_new (&dbp->ddl))
    {
      fprintf (stderr, "failed to create debug pass draw list\n");
      return 1;
    }

  if (load_shaders (dbp))
    return 1;

  if (create_pipeline_layout (dbp))
    return 1;

  if (create_pipeline (dbp, rp))
    return 1;

  return 0;
}

void
debug_pass_delete (debug_pass_t *dbp)
{
  if (dbp->pipeline)
    vkDestroyPipeline (dbp->vkd, dbp->pipeline, NULL);

  if (dbp->pipeline_layout)
    vkDestroyPipelineLayout (dbp->vkd, dbp->pipeline_layout, NULL);

  if (dbp->vertex_shader)
    gpu_shader_delete (dbp->vertex_shader);

  if (dbp->fragment_shader)
    gpu_shader_delete (dbp->fragment_shader);

  if (dbp->ddl)
    debug_draw_list_delete (dbp->ddl);

  free (dbp);
}

int
debug_frame_data_init (debug_pass_t *dbp, struct debug_frame_data *frame)
{
  frame->vertices = NULL;
  frame->indices = NULL;

  const VkBufferUsageFlags VERTEX_USAGE = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  const VkBufferUsageFlags INDEX_USAGE = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  if (gpu_vector_new (&frame->vertices, dbp->gpu, VERTEX_USAGE))
    {
      fprintf (stderr, "failed to create vertex buffer\n");
      return 1;
    }

  if (gpu_vector_new (&frame->indices, dbp->gpu, INDEX_USAGE))
    {
      fprintf (stderr, "failed to create index buffer\n");
      return 1;
    }

  return 0;
}

void
debug_frame_data_cleanup (debug_pass_t *dbp, struct debug_frame_data *frame)
{
  if (frame->vertices)
    gpu_vector_delete (frame->vertices);

  if (frame->indices)
    gpu_vector_delete (frame->indices);
}

void
debug_pass_render (debug_pass_t *dbp, const struct render_context *ctx,
                   struct debug_frame_data *frame)
{
  /* temporary debug draw */
  {
    debug_draw_vertex_t vertex1 = {
      .color = { 1.0, 0.0, 0.0 },
      .position = { 0.5, 0.5, 0.0 },
    };

    debug_draw_vertex_t vertex2 = {
      .color = { 0.0, 1.0, 0.0 },
      .position = { 0.5, -0.5, 0.0 },
    };

    debug_draw_index_t index1 = debug_draw_list_vertex (dbp->ddl, &vertex1);
    debug_draw_index_t index2 = debug_draw_list_vertex (dbp->ddl, &vertex2);

    debug_draw_list_line (dbp->ddl, index1, index2);
  }

  frame->vertex_num = debug_draw_list_vertex_num (dbp->ddl);
  frame->index_num = debug_draw_list_index_num (dbp->ddl);

  const debug_draw_vertex_t *vertices = debug_draw_list_vertices (dbp->ddl);
  const debug_draw_index_t *indices = debug_draw_list_indices (dbp->ddl);

  gpu_vector_write (frame->vertices, vertices, sizeof (debug_draw_vertex_t),
                    frame->vertex_num);
  gpu_vector_write (frame->indices, indices, sizeof (debug_draw_index_t),
                    frame->index_num);

  debug_draw_list_clear (dbp->ddl);

  VkBuffer vertex_buffer = gpu_vector_get (frame->vertices);
  VkBuffer index_buffer = gpu_vector_get (frame->indices);

  vkCmdBindPipeline (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dbp->pipeline);

  size_t offsets[] = { 0 };
  vkCmdBindVertexBuffers (ctx->cmd, 0, 1, &vertex_buffer, offsets);
  vkCmdBindIndexBuffer (ctx->cmd, index_buffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed (ctx->cmd, frame->index_num, 1, 0, 0, 0);
}
