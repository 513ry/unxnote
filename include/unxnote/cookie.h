#ifndef _COOKIE_H
#define _COOKIE_H

#include "common.h"

/*
 * Window Cookie Class
 */
typedef struct {
  uint32_t glyph;
  char *title;
  uint32_t fg_color;
  uint32_t bg_color;
} UNXNoteCookie;

#endif // _COOKIE_H
