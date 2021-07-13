/** @file network_server.c
 */

#include "network/network_server.h"

#include "log.h"

/* TODO(marceline-cramer): custom mem alloc */
#include <stdlib.h> /* for mem alloc */

#include <uv.h>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 10555
#define DEFAULT_BACKLOG 10556

struct network_server_s
{
  uv_loop_t loop;

  /* connection state */
  uv_tcp_t socket;
};

static void
on_connection (uv_stream_t *req, int status)
{
  network_server_t *server = req->data;
  LOG_MSG ("received connection request\nstatus: %d", status);

  if (status >= 0)
    {
      LOG_INF ("accepting connection...");

      uv_tcp_t client;
      uv_tcp_init (&server->loop, &client);
      uv_accept (req, (uv_stream_t *)&client);
    }
}

static int
server_listen (network_server_t *server)
{
  const char *ip_address = DEFAULT_IP;
  int port = DEFAULT_PORT;
  int backlog = DEFAULT_BACKLOG;

  LOG_MSG ("listening on: %s:%d (backlog %d)", ip_address, port, backlog);

  struct sockaddr_in addr;
  uv_ip4_addr (ip_address, port, &addr);
  uv_tcp_bind (&server->socket, (const struct sockaddr *)&addr, 0);

  int r = uv_listen ((uv_stream_t *)&server->socket, backlog, on_connection);

  if (r)
    {
      LOG_ERR ("listen error: %s", uv_strerror (r));
      return 1;
    }

  LOG_MSG ("successfully listening");
  return 0;
}

int
network_server_new (network_server_t **new_server)
{
  network_server_t *server = malloc (sizeof (network_server_t));
  *new_server = server;

  /* TODO(marceline-cramer): error handling? */
  uv_loop_init (&server->loop);

  uv_tcp_init (&server->loop, &server->socket);
  server->socket.data = server;

  if (server_listen (server))
    {
      LOG_ERR ("failed to listen");
      return 1;
    }

  return 0;
}

void
network_server_delete (network_server_t *server)
{
  uv_close ((uv_handle_t *)&server->socket, NULL);
  uv_loop_close (&server->loop);

  free (server);
}

void
network_server_update (network_server_t *server)
{
  uv_run (&server->loop, UV_RUN_NOWAIT);
}
