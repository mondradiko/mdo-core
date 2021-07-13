/*! @file log.h
  @brief Provides functions to display user-friendly logging messages.
  @copyright Copyright (c) 2021 Marceline Cramer
  SPDX-License-Identifier: MIT
 */
#pragma once

typedef enum mdo_log_level_e
{
  MDO_LOG_ZONE = 0,
  MDO_LOG_INFO,
  MDO_LOG_DEBUG,
  MDO_LOG_MESSAGE,
  MDO_LOG_WARNING,
  MDO_LOG_ERROR
} mdo_log_level_t;

/*! @function log_at
  @param file
  @param line
  @param level
  @param format
 */

#define LOG_AT(lvl, ...) log_at (__FILE__, __LINE__, lvl, __VA_ARGS__)
#define LOG_INF(...) LOG_AT (MDO_LOG_INFO, __VA_ARGS__)
#define LOG_DBG(...) LOG_AT (MDO_LOG_DEBUG, __VA_ARGS__)
#define LOG_MSG(...) LOG_AT (MDO_LOG_MESSAGE, __VA_ARGS__)
#define LOG_WRN(...) LOG_AT (MDO_LOG_WARNING, __VA_ARGS__)
#define LOG_ERR(...) LOG_AT (MDO_LOG_ERROR, __VA_ARGS__)

#define DEFAULT_PORT 0080
#define DEFAULT_BACKLOG 0083

void on_new_connection(uv_stream_t *server, int status);
void close_connection();

MDO_EXPORT void log_at (const char *, int, mdo_log_level_t, const char *, ...);
