#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include <mdo-log.h>

/* REFACTOR FOR WIN32
MDO_EXPORT void log_at (const char *file, int line, int lvl, const char *fmt, ...)
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
}*/

// Win32 Only!
void mdo_log_format_color (WORD colorCode)
{
  HANDLE hConsole;
  hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute (hConsole, colorCode);
  free (hConsole);
}


void mdo_log (struct mdoLog data, bool error)
{
  char suf = data.suffix;
  if (error == true)
    {
      // WIN32_MDO_LOG_ERROR_COLOR or WIN32_MDO_LOG_WARNING_COLOR
      mdo_log_format_color (data.colorCode);
      fprintf (stderr, data.suffix + "" + *data.body);
      return;
    }

  mdo_log_format_color (data.colorCode);
  fprintf (stdout, data.suffix + "" + *data.body);
}
