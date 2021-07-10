/*! @file
 */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "export.h"

static const char *log_color (mdo_log_level_t level)
{
  switch (level)
    {
    case MDO_LOG_ZONE:
      return "\x1b[33m";
    case MDO_LOG_INFO:
      return "\x1b[94m";
    case MDO_LOG_DEBUG:
      return "\x1b[32m";
    case MDO_LOG_MESSAGE:
      return "\x1b[96m";
    case MDO_LOG_WARNING:
      return "\x1b[33m";
    default:
      return "\x1b[31m";
    }
}

static const char *log_prefix (mdo_log_level_t level)
{
  switch (level)
    {
    case MDO_LOG_ZONE:
      return "╔(ZNE)";
    case MDO_LOG_INFO:
      return "╔(INF)";
    case MDO_LOG_DEBUG:
      return "╔(DBG)";
    case MDO_LOG_MESSAGE:
      return "╔(MSG)";
    case MDO_LOG_WARNING:
      return "╔(WRN)";
    default:
      return "╔(ERR)";
    }
}

MDO_EXPORT void log_at (const char *file, int line, mdo_log_level_t lvl, const char *fmt, ...)
{
  FILE *stream = stdout;

  const char *color = log_color (lvl);
  const char *prefix = log_prefix (lvl);
  fprintf (stream, "%s%s@[%s:%d]\x1b[0m\n", color, prefix, file, line);

  char message[512];

  va_list args;
  va_start (args, fmt);
  int len = vsnprintf (message, sizeof (message), fmt, args);
  va_end (args);

  size_t idx = 0;
  size_t line_len = 0;
  const char *last_line = message;

  while (idx <= len)
    {
      char c = message[idx];

      if (c == '\n' || c == '\0')
        {
          const char *bracket = "╚";
          const char *this_line = &message[idx];

          if (c == '\n')
            {
              bracket = "╠";
              this_line++;

              if (last_line != message)
                line_len--;
            }

          char line[256];
          memcpy (line, last_line, line_len);
          line[line_len] = '\0';

          fprintf (stream, "%s%s %s\x1b[0m\n", color, bracket, line);

          line_len = 0;
          last_line = this_line;
        }

      ++idx;
      ++line_len;
    }
}
