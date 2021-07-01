/** @file render_phases.h
 */

#pragma once

#include "renderer/camera.h"

#define MAX_FRAMES_IN_FLIGHT 4

struct render_context
{
  VkCommandBuffer cmd;
  camera_t *camera;
};
