/** @file camera.h
 */

#pragma once

/** @typedef camera_t
 */
typedef struct camera_s camera_t;

/** @function camera_new
 */
int camera_new (camera_t **);

/** @function camera_delete
 */
void camera_delete (camera_t *);
