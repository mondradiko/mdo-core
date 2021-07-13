/*! @file log.h
  @brief Provides functions to display user-friendly logging messages.
  @copyright Copyright (c) 2021 Marceline Cramer
  SPDX-License-Identifier: MIT
 */
#pragma once

#define DEFAULT_PORT 80
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_BACKLOG 83

void bind_interface_addresses()
void on_new_connection(uv_stream_t *server, int status);
void close_connection();