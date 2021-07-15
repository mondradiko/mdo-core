/** @file network_client.h
 */

#pragma once

/** @typedef network_client_t
 */
typedef struct network_client_s network_client_t;

/** @function network_client_new
 */
int network_client_new (network_client_t **);

/** @function network_client_delete
 */
void network_client_delete (network_client_t *);

/** @function network_client_connect
 */
int network_client_connect (network_client_t *, const char *, int);

/** @function network_client_update
 */
void network_client_update (network_client_t *);
