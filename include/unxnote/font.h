/** font.h
 * Copyright (c) 2024, Daniel Sierpiński All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - All advertising materials mentioning features or use of this software must
 *   display the following acknowledgement: This product includes software
 *   developed by the Daniel Sierpiński.
 * - Neither the name of the Daniel Sierpiński nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DANIEL SIERPIŃSKI AS IS AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL DANIEL SIERPIŃSKI BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * DESCRIPTION:
 * Draw UTF8 fonts in most formats. 󱣻
 */

#ifndef _FONT_H
#define _FONT_H

#include "common.h"

typedef uint32_t xcb_gcontext_t;
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef struct FT_FaceRec_ *FT_Face;

#define FONT_SIZE UINT32_C(16)

/**
 * Initialize font
 */
bool init_font();

/**
 * Create a new font from file
 */
FT_Face new_font(char *font_path);

/**
 * Free library and all allocated font faces
 */
void free_font(unsigned int, ...);

/**
 * Draw FT font using XCB graphical context
 */
void draw_text(xcb_connection_t *conn, xcb_window_t window, xcb_gcontext_t gc,
	       FT_Face face, const char *text, uint32_t x, uint32_t y);

/**
 * Encode glyph nubler to UTF8 characters
 */
const char *utf8_encode(uint32_t glyph);

#define _CHAR_AT(INDEX, ...)				\
  ((__VA_ARGS__ >> (uint64_t)8 * INDEX & 0xff) << INDEX * 8)

#define _CHAR_FOUR(...)	                                \
  _CHAR_AT(0, __VA_ARGS__) |				\
  _CHAR_AT(1, __VA_ARGS__) |				\
  _CHAR_AT(2, __VA_ARGS__) |				\
  _CHAR_AT(3, __VA_ARGS__) |				\
  _CHAR_AT(4, __VA_ARGS__)

#define HEX2UTF8(...)					\
  ({uint64_t tmp = _CHAR_FOUR((uint64_t)__VA_ARGS__); &tmp;})

#endif // _FONT_H
