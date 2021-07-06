/** @file world.c
 */

#include "world/world.h"
#include "renderer/debug/debug_draw.h"

#include <math.h>

#include <TracyC.h>
#include <flecs.h>
#include <flecs/modules/system.h>

struct world_s
{
  ecs_world_t *ecs;

  ecs_entity_t spin;
  ecs_entity_t draw;
};

typedef struct transform_component_s
{
  float position[2];
} transform_component_t;

typedef struct spin_component_s
{
  float phase;
  float radius;
  float speed;
} spin_component_t;

typedef struct color_component_s
{
  float color[3];
} color_component_t;

void
spin (ecs_iter_t *it)
{
  TracyCZone (ctx, true);

  transform_component_t *ts = ecs_term (it, transform_component_t, 1);
  spin_component_t *ss = ecs_term (it, spin_component_t, 2);

  for (int i = 0; i < it->count; i++)
    {
      transform_component_t *t = &ts[i];
      spin_component_t *s = &ss[i];

      s->phase += it->delta_time * s->speed;
      t->position[0] = cos (s->phase) * s->radius;
      t->position[1] = sin (s->phase) * s->radius;
    }

  TracyCZoneEnd (ctx);
}

static debug_draw_index_t
make_vertex (debug_draw_list_t *ddl, transform_component_t *transform,
             color_component_t *color, float xoff, float yoff)
{
  float x = transform->position[0] + xoff;
  float y = transform->position[1] + yoff;

  debug_draw_vertex_t vertex = {
    .position = { x, y, 0.0 },
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
      debug_draw_index_t v1 = make_vertex (ddl, t, c, -r, -r);
      debug_draw_index_t v2 = make_vertex (ddl, t, c, -r, r);
      debug_draw_index_t v3 = make_vertex (ddl, t, c, r, -r);
      debug_draw_index_t v4 = make_vertex (ddl, t, c, r, r);

      debug_draw_list_line (ddl, v1, v4);
      debug_draw_list_line (ddl, v2, v3);
    }

  TracyCZoneEnd (ctx);
}

static float
norm_rand ()
{
  return ((float)rand ()) / RAND_MAX;
}

static void
randomize_transform (transform_component_t *t)
{
  t->position[0] = (norm_rand () * 2.0 - 1.0) * 0.9;
  t->position[1] = (norm_rand () * 2.0 - 1.0) * 0.9;
}

static void
randomize_spin (spin_component_t *s)
{
  s->radius = norm_rand () * 0.9 + 0.05;
  s->speed = norm_rand () * 4.0 - 2.0;
  s->speed = s->speed / (s->radius + 1.0);
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
  float h = norm_rand ();
  float s = 1.0;
  float v = 1.0;

  hsv_to_rgb (c->color, h, s, v);
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
    .entity.name = "Spin",
    .size = sizeof (spin_component_t),
    .alignment = ECS_ALIGNOF (spin_component_t),
  };

  ecs_component_desc_t c_desc = {
    .entity.name = "Color",
    .size = sizeof (color_component_t),
    .alignment = ECS_ALIGNOF (color_component_t),
  };

  ecs_entity_t t_c = ecs_component_init (w->ecs, &t_desc);
  ecs_entity_t s_c = ecs_component_init (w->ecs, &s_desc);
  ecs_entity_t c_c = ecs_component_init (w->ecs, &c_desc);

  for (int i = 0; i < 25000; i++)
    {
      ecs_entity_t e = ecs_new_id (w->ecs);
      randomize_transform (ecs_get_mut_w_id (w->ecs, e, t_c, NULL));
      randomize_spin (ecs_get_mut_w_id (w->ecs, e, s_c, NULL));
      randomize_color (ecs_get_mut_w_id (w->ecs, e, c_c, NULL));
    }

  ecs_system_desc_t spin_desc = {
    .entity = (ecs_entity_desc_t){
      .name = "spin",
      .add = EcsOnUpdate,
    },
    .query.filter.expr = "Transform, Spin",
    .callback = spin,
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
