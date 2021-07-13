/*! @file log.h
  @brief Provides functions to display user-friendly logging messages.
  @copyright Copyright (c) 2021 Marceline Cramer
  SPDX-License-Identifier: MIT
 */

#pragma once
//#include <winsock.h>
//#include <sys/socket.h>

void makeConnection(char[]* uv_ip, int uv_port);
void pingDNS(char[]* dns_Namespace);
void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);