/** @file network_client.c
 */

#include "network/network_client.h"

#include "log.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

#include <uv.h>

struct network_client_s
{
  uv_loop_t loop;

  /* connection info */
  uv_connect_t connect;
  uv_tcp_t socket;
};

int
network_client_new (network_client_t **new_client)
{
  network_client_t *client = malloc (sizeof (network_client_t));
  *new_client = client;

  /* TODO(marceline-cramer): error handling? */
  uv_loop_init (&client->loop);
  uv_tcp_init (&client->loop, &client->socket);

  return 0;
}

void
network_client_delete (network_client_t *client)
{
  uv_close ((uv_handle_t *)&client->socket, NULL);
  uv_loop_close (&client->loop);

  free (client);
}

static void
on_connect (uv_connect_t *req, int status)
{
  LOG_MSG ("connect status: %d", status);
}

int
network_client_connect (network_client_t *client, const char *address,
                        int port)
{
  LOG_MSG ("connecting to: %s:%d", address, port);

  struct sockaddr_in dest;
  uv_ip4_addr (address, port, &dest);

  client->connect = (uv_connect_t){
    .data = client,
  };

  /* TODO(marceline-cramer): error handling? */
  uv_tcp_connect (&client->connect, &client->socket,
                  (const struct sockaddr *)&dest, on_connect);

  return 0;
}

void
network_client_update (network_client_t *client)
{
  uv_run (&client->loop, UV_RUN_NOWAIT);
}
