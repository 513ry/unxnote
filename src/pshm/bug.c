#include "unxnote-common/bug.h"
#if !defined(UNX_NO_STDIO)
#include <stdio.h>
#endif
#include <stdarg.h>

// #include "unxnote/state.h"

void
unxnote_log(const char *msg, ...)
{
#if !defined(UNX_NO_STDIO)
  va_list ap;
  char *str;
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
#endif
}

void
unxnote_bug(const char *msg, int eno)
{
#if !defined(UNX_NO_STDIO)
  fputs("UNXNote failed: ", stderr);
  fputs(msg, stderr);
  fputs("\n", stderr);
#endif
  exit(eno | EXIT_FAILURE);
}
