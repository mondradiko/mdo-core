/** @file network_client.h
 */

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): custom allocation */
#include <stdlib.h> /* for mem alloc */
#include <string.h> /* for memcpy */

#include "uv.h"
#include "network_client.h"


void makeConnection(char[]* uv_ip, int uv_port){
  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);

  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

  struct sockaddr_in dest;
  //"127.0.0.1" 80 uv_ip, uv_port
  uv_ip4_addr("127.0.0.1", 80, &dest);
  uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);
}

//basis to connect to a dns server
void pingDNS(char[]* dns_namespace){
   loop = uv_default_loop();

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t resolver;
    fprintf(stderr, dns_namespace " is... ");
    int r = uv_getaddrinfo(loop, &resolver, on_resolved, *dns_namespace, "6667", &hints);

    if (r) {
        fprintf(stderr, "getaddrinfo call error %s\n", uv_err_name(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}

//basis function to read packets
void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) req, NULL);
        free(buf->base);
        return;
    }

    char sender[17] = { 0 };
    uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
    fprintf(stderr, "Recv from %s\n", sender);

    // ... DHCP specific code
    unsigned int *as_integer = (unsigned int*)buf->base;
    unsigned int ipbin = ntohl(as_integer[4]);
    unsigned char ip[4] = {0};
    int i;

    for (i = 0; i < 4; i++){
        ip[i] = (ipbin >> i*8) & 0xff;
    }

    fprintf(stderr, "Offered IP %d.%d.%d.%d\n", ip[3], ip[2], ip[1], ip[0]);

    free(buf->base);
    uv_udp_recv_stop(req);
}