/** window.h
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
 * Determines screen geometry and draws window on aviable region. 󰟆
 *
 * TODO: Consider changing window.h to screen.h
 */

#if !defined(_WINDOW_H)
#     define _WINDOW_H

#include "unxnote/common.h"

#define WINDOW_MARGIN UINT32_C(10)
#define WINDOW_PADDING UINT32_C(3)
#define WINDOW_WIDTH UINT32_C(320)
#define WINDOW_MIN_HEIGHT UINT32_C(100) /* Height will inflate by number of
					   lines * FONT_SIZE + header */

typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_visualid_t;
typedef uint32_t xcb_window_t;

/*
 * Structure with pointers to xcb types
 */

typedef struct {
  xcb_connection_t *conn;
  uint32_t screen_width;
  uint32_t screen_height;
  uint32_t screen_x;
  uint32_t screen_y;
  xcb_window_t screen_root;
  xcb_visualid_t screen_visualid;
  uint32_t mask[2];
} XConnection;

extern XConnection x_conn;

/**
 * Initialize XCB connection
 *
 * ERRNO codes:
 * EACCES    - XCB connection has error
 * ECANCELED - Retriving primary monitor failed
 *
 * @return true on success and false on any error
 */

bool init_window();

/**
 * Create a new window
 *
 * @param id Window's id determines it's precedence and position
 * @return XCB window
 */

xcb_window_t new_window(size_t id);

/**
 * Free XCB connection
 *
 * @return true on success and false on any error
 */

void free_window();

#endif // _WINDOW_H
