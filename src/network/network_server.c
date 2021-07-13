/** @file network_server.h
 */


/* TODO(marceline-cramer): custom logging check out mdo-log*/
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): custom allocation */
#include <stdlib.h> /* for mem alloc */
#include <string.h> /* for memcpy */
#include <netinet/in.h>

#include <network_server.h>
#include <uv.h>

void bind_interface_addresses(){
    char buf[512];
    uv_interface_address_t *info;
    int count, i;

    uv_interface_addresses(&info, &count);
    i = count;

    printf("Number of interfaces: %d\n", count);
    while (i--) {
        uv_interface_address_t interface = info[i];

        printf("Name: %s\n", interface.name);
        printf("Internal? %s\n", interface.is_internal ? "Yes" : "No");
        
        if (interface.address.address4.sin_family == AF_INET) {
            uv_ip4_name(&interface.address.address4, buf, sizeof(buf));
            printf("IPv4 address: %s\n", buf);
        }
        else if (interface.address.address4.sin_family == AF_INET6) {
            uv_ip6_name(&interface.address.address6, buf, sizeof(buf));
            printf("IPv6 address: %s\n", buf);
        }

        printf("\n");
    }

    uv_free_interface_addresses(info, count);
    return 0;
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
}


void close_connection(){
  uv_close((uv_handle_t*) client, on_close);
  free(buf->base);
}

int main() {
    loop = uv_default_loop();
    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    char str[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

    //get user's ipv4 addr
    printf("User IP address is: %s\n", inet_ntoa(ipAddr)); //client_addr.sin_addr));
    printf("Port is: %d\n", (int) ntohs(pV4Addr.sin_port)); //client_addr.sin_port));

    //pass users addr
    //uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    /* IPv6 Func
        struct sockaddr_in6* pV6Addr = (struct sockaddr_in6*)&client_addr;
        struct in6_addr ipAddr       = pV6Addr->sin6_addr;
        and getting a string is almost identical to IPV4

        char str[INET6_ADDRSTRLEN];
        inet_ntop( AF_INET6, &ipAddr, str, INET6_ADDRSTRLEN );
    */

    uv_ip4_addr(ipAddr, pV4Addr.sin_port, &pV4Addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&pV4Addr, 0);
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);

    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    else {
        fprintf(stdout, "Connection Established, Listening Successfully!");
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}