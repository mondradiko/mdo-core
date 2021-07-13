/** @file network_server.
 */

#pragma once

/** @typedef network_server_t
 */
typedef struct network_server_s network_server_t;

/** @function network_server_new
 */
int network_server_new (network_server_t **);

/** @function network_server_delete
 */
void network_server_delete (network_server_t *);

/** @function network_server_update
 */
void network_server_update (network_server_t *);
