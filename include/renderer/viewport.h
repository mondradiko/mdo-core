/** @file viewport.h
 */

#pragma once

/** @typedef viewport_t
 */
typedef struct viewport_s viewport_t;

struct viewport_config_t
{
};

/** @function viewport_new
 */
int viewport_new (viewport_t **, const struct viewport_config_t *);

/** @function viewport_delete
 */
void viewport_delete (viewport_t *);
