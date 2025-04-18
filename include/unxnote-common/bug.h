#ifndef _BUG_H
#define _BUG_H

#include <stdlib.h>

/**
 * Log a message to /tmp/unxnote.log and/or stderr, depending if UNX_LOG and
 * UNX_NO_STDIO are defined.
 */
void unxnote_log(const char *msg, ...);

/**
 * Clean memory and exit with message and error number.
 */
void unxnote_bug(const char *msg, int eno);

#endif // _BUG_H
