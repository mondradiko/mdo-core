#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define MAX_EXTENSIONS 32
#define MAX_PHYSICAL_DEVICES 32
#define MAX_QUEUE_FAMILIES 32

// UNIX
#define ANSI_COLOR_RESET "\x1b[0m"
#define MDO_LOG_ZONE_COLOR "\x1b[33m"
#define MDO_LOG_INFO_COLOR "\x1b[94m"
#define MDO_LOG_DEBUG_COLOR "\x1b[32m"
#define MDO_LOG_MESSAGE_COLOR "\x1b[96m"
#define MDO_LOG_WARNING_COLOR "\x1b[33m"
#define MDO_LOG_ERROR_COLOR "\x1b[31m"

#define U_MDO_LOG_ZONE "╔(ZNE)"
#define U_MDO_LOG_INFO "╔(INF)"
#define U_MDO_LOG_DEBUG "╔(DBG)"
#define U_MDO_LOG_MESSAGE "╔(MSG)"
#define U_MDO_LOG_WARNING "╔(WRN)"
#define U_MDO_LOG_ERROR "╔(ERR)"

#define MDO_LOG_BEGIN "╔"
#define MDO_LOG_MIDDLE "╠"
#define MDO_LOG_END "╚"

// WIN
#define WIN32_MDO_LOG_ZONE_COLOR 0x0000
#define WIN32_MDO_LOG_INFO_COLOR 0x000D
#define WIN32_MDO_LOG_DEBUG_COLOR 0x0002
#define WIN32_MDO_LOG_MESSAGE_COLOR 0x000B
#define WIN32_MDO_LOG_WARNING_COLOR 0x0006
#define WIN32_MDO_LOG_ERROR_COLOR 0x0004

#define MDO_LOG_ZONE "空(ZNE)"
#define MDO_LOG_INFO "信(INF)"
#define MDO_LOG_DEBUG "修(DBG)"
#define MDO_LOG_MESSAGE "小(MSG)"
#define MDO_LOG_WARNING "魏(WRN)"
#define MDO_LOG_ERROR "顶(ERR)";

#define UNIX "__linux__"

#if defined(WIN32)
#define MDO_EXPORT __declspec(dllexport)
#elif defined(UNIX)
#define MDO_EXPORT __attribute__ ((visibility ("default")))
#else
#define MDO_EXPORT
#endif

//new mdo_log struct for mdo_log function

struct mdoLog{
  char suffix;
  WORD colorCode;
  char *body;
  int errId;
};

// Win32 Only!
void mdo_log_format_color (WORD colorCode);
void mdo_log (struct mdoLog data, bool error);
MDO_EXPORT void log_at (const char *, int, int, const char *, ...);
