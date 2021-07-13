/** @file network_client.h
 */

/* TODO(marceline-cramer): custom logging */
#include <stdio.h> /* for fprintf */
/* TODO(marceline-cramer): custom allocation */
#include <stdlib.h> /* for mem alloc */
#include <string.h> /* for memcpy */
#include <uv.h>

int establishConnection(){
  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);

  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

  //mdo-server IP, Static?
  struct sockaddr_in dest;
  //needs exposed ip
  struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;

  char str[INET_ADDRSTRLEN];
  inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

  //get user's ipv4 addr
  printf("User IP address is: %s\n", inet_ntoa(ipAddr)); //client_addr.sin_addr));
  printf("Port is: %d\n", (int) ntohs(pV4Addr.sin_port)); //client_addr.sin_port));

  uv_ip4_addr(ipAddr, pV4Addr.sin_port, &dest); //&pV4Addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&pV4Addr, 0);

  //uv_ip4_addr("127.0.0.1", 80, &dest);
  uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);
}

