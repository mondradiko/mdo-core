/** @file debug.vert
 */

#version 450

layout (set = 0, binding = 0) uniform ViewportUniform
{
  mat4 projection_mat;
  mat4 view_mat;
} viewport;

layout (location = 0) in vec3 vert_position;
layout (location = 1) in vec3 vert_color;

layout (location = 0) out vec3 frag_color;

void
main ()
{
  gl_Position = viewport.projection_mat * viewport.view_mat * vec4 (vert_position, 1.0);
  frag_color = vert_color;
}
