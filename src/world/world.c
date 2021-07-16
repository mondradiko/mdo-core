/** @file world.c
 */

#include "world/world.h"
#include "renderer/debug/debug_draw.h"

#include <math.h>

#include <TracyC.h>
#include <cglm/vec3.h>
#include <flecs.h>

static const vec3 BLACK_HOLE_POSITION = { 0.0, 0.0, 0.0 };
static const float BLACK_HOLE_MASS = 10000.0;

struct world_s
{
  ecs_world_t *ecs;

  ecs_entity_t spin;
  ecs_entity_t draw;
};

typedef struct transform_component_s
{
  float position[3];
} transform_component_t;

typedef struct star_component_s
{
  float velocity[3];
  float mass;
} star_component_t;

typedef struct color_component_s
{
  float color[3];
} color_component_t;

void
orbit (ecs_iter_t *it)
{
  TracyCZone (ctx, true);

  transform_component_t *ts = ecs_term (it, transform_component_t, 1);
  star_component_t *ss = ecs_term (it, star_component_t, 2);

  for (int i = 0; i < it->count; i++)
    {
      transform_component_t *t = &ts[i];
      star_component_t *s = &ss[i];

      vec3 gravity_dir;
      glm_vec3_sub (BLACK_HOLE_POSITION, t->position, gravity_dir);

      static const float G = 0.0001;
      float r2 = glm_vec3_norm2 (gravity_dir);
      float m1m2 = BLACK_HOLE_MASS * s->mass;
      float gravity_scale = G * m1m2 / r2;

      vec3 gravity_force;
      glm_vec3_scale (gravity_dir, gravity_scale, gravity_force);

      /* F = ma */
      vec3 acceleration;
      glm_vec3_scale (gravity_force, 1.0 / s->mass, acceleration);

      vec3 velocity;
      glm_vec3_add (acceleration, s->velocity, velocity);

      glm_vec3_copy (velocity, s->velocity);

      glm_vec3_muladds (velocity, it->delta_time, t->position);
    }

  TracyCZoneEnd (ctx);
}

static debug_draw_index_t
make_vertex (debug_draw_list_t *ddl, transform_component_t *transform,
             color_component_t *color, float xoff, float yoff, float zoff)
{
  float x = transform->position[0] + xoff;
  float y = transform->position[1] + yoff;
  float z = transform->position[2] + zoff;

  debug_draw_vertex_t vertex = {
    .position = { x, y, z },
    .color = { color->color[0], color->color[1], color->color[2] },
  };

  return debug_draw_list_vertex (ddl, &vertex);
}

void
draw (ecs_iter_t *it)
{
  TracyCZone (ctx, true);

  transform_component_t *ts = ecs_term (it, transform_component_t, 1);
  color_component_t *cs = ecs_term (it, color_component_t, 2);
  debug_draw_list_t *ddl = it->ctx;

  for (int i = 0; i < it->count; i++)
    {
      transform_component_t *t = &ts[i];
      color_component_t *c = &cs[i];

      float r = 0.01;
      debug_draw_index_t v1, v2;

      v1 = make_vertex (ddl, t, c, r, 0.0, 0.0);
      v2 = make_vertex (ddl, t, c, -r, 0.0, 0.0);
      debug_draw_list_line (ddl, v1, v2);

      /*v1 = make_vertex (ddl, t, c, 0.0, r, 0.0);
      v2 = make_vertex (ddl, t, c, 0.0, -r, 0.0);
      debug_draw_list_line (ddl, v1, v2);

      v1 = make_vertex (ddl, t, c, 0.0, 0.0, r);
      v2 = make_vertex (ddl, t, c, 0.0, 0.0, -r);
      debug_draw_list_line (ddl, v1, v2);*/
    }

  TracyCZoneEnd (ctx);
}

static float
norm_rand ()
{
  return ((float)rand ()) / RAND_MAX;
}

static float
snorm_rand ()
{
  return norm_rand () * 2.0 - 1.0;
}

static void
randomize_star (transform_component_t *t, star_component_t *s,
                color_component_t *c)
{
  float radius = 5.0;
  float offset = radius / 2.0;
  float repel = 3.0;
  t->position[0] = (norm_rand () * radius) - offset;
  t->position[1] = (norm_rand () * radius) - offset;
  t->position[2] = (norm_rand () * radius) - offset;

  vec3 sign;
  glm_vec3_sign (t->position, sign);
  glm_vec3_scale (sign, repel, sign);
  glm_vec3_add (sign, t->position, t->position);

  s->mass = norm_rand () * 1000.0 + 400.0;

  float velocity = 1000.0 / s->mass;
  s->velocity[0] = snorm_rand ();
  s->velocity[1] = snorm_rand ();
  s->velocity[2] = snorm_rand ();

  vec3 orbit_dir;
  glm_vec3_sub (BLACK_HOLE_POSITION, t->position, orbit_dir);
  glm_vec3_cross (orbit_dir, s->velocity, s->velocity);
  glm_vec3_normalize (s->velocity);
  glm_vec3_scale (s->velocity, velocity, s->velocity);

  /* rainbow */
  /*float h = norm_rand ();
  float s = 1.0;
  float v = 1.0;

  hsv_to_rgb (c->color, h, s, v);*/

  /* white */
  /*glm_vec3_zero (c->color);
  glm_vec3_adds (c->color, 1.0, c->color);*/

  /* naive mock blackbody radiation */
  float temperature = norm_rand ();

  c->color[0] = (temperature + 8.0) / (temperature * 10.0 + 1.0);
  c->color[1] = pow (temperature, 2.0);
  c->color[2] = pow (temperature, 6.0);
}

static void
hsv_to_rgb (float rgb[3], float h, float s, float v)
{
  int i = floor (h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  float r, g, b;
  switch (i % 6)
    {
    case 0:
      r = v, g = t, b = p;
      break;
    case 1:
      r = q, g = v, b = p;
      break;
    case 2:
      r = p, g = v, b = t;
      break;
    case 3:
      r = p, g = q, b = v;
      break;
    case 4:
      r = t, g = p, b = v;
      break;
    case 5:
      r = v, g = p, b = q;
      break;
    }

  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

static void
randomize_color (color_component_t *c)
{
}

int
world_new (world_t **new_w, debug_draw_list_t *ddl)
{
  world_t *w = malloc (sizeof (world_t));
  *new_w = w;

  w->ecs = ecs_init ();

  ecs_component_desc_t t_desc = {
    .entity.name = "Transform",
    .size = sizeof (transform_component_t),
    .alignment = ECS_ALIGNOF (transform_component_t),
  };

  ecs_component_desc_t s_desc = {
    .entity.name = "Star",
    .size = sizeof (star_component_t),
    .alignment = ECS_ALIGNOF (star_component_t),
  };

  ecs_component_desc_t c_desc = {
    .entity.name = "Color",
    .size = sizeof (color_component_t),
    .alignment = ECS_ALIGNOF (color_component_t),
  };

  ecs_entity_t t_c = ecs_component_init (w->ecs, &t_desc);
  ecs_entity_t s_c = ecs_component_init (w->ecs, &s_desc);
  ecs_entity_t c_c = ecs_component_init (w->ecs, &c_desc);

  for (int i = 0; i < 1000; i++)
    {
      ecs_entity_t e = ecs_new_id (w->ecs);

      transform_component_t *t = ecs_get_mut_w_id (w->ecs, e, t_c, NULL);
      star_component_t *s = ecs_get_mut_w_id (w->ecs, e, s_c, NULL);
      color_component_t *c = ecs_get_mut_w_id (w->ecs, e, c_c, NULL);

      randomize_star (t, s, c);
    }

  ecs_system_desc_t spin_desc = {
    .entity = (ecs_entity_desc_t){
      .name = "orbit",
      .add = EcsOnUpdate,
    },
    .query.filter.expr = "Transform, Star",
    .callback = orbit,
  };

  ecs_system_init (w->ecs, &spin_desc);

  ecs_system_desc_t draw_desc = {
    .entity = (ecs_entity_desc_t){
      .name = "draw",
      .add = EcsOnUpdate,
    },
    .query.filter.expr = "Transform, Color",
    .ctx = ddl,
    .callback = draw,
  };

  ecs_system_init (w->ecs, &draw_desc);

  return 0;
}

void
world_delete (world_t *w)
{
  ecs_fini (w->ecs);

  free (w);
}

void
world_step (world_t *w, float dt)
{
  ecs_progress (w->ecs, dt);
}
