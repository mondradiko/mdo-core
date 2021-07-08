/** @file viewport_uniform.h
 */

#pragma once

#include <cglm/cglm.h>

/** @typedef viewport_uniform_t
 */
typedef struct viewport_uniform_s
{
  mat4 projection_mat;
  mat4 view_mat;
} viewport_uniform_t;
