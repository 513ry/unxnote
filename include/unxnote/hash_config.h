#ifndef _HASH_CONFIG_H
#define _HASH_CONFIG_H

#include <unxnote/cookie.h>
#include <xcb/xproto.h>

/**
 * Custom Hash value types
 */
union HashValue {
  UNXNoteCookie cookie;
  xcb_gcontext_t contexts[2];
};

#define HASH_UNION_TYPE union HashValue

#endif
