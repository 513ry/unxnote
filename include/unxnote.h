/** unxnote.h
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
 * Include header for unxnote C API. 
 */

#ifndef _UNXNOTE_H
#define _UNXNOTE_H

/* Target C99 SUSv2 source */

#ifdef __cplusplus
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#define _XOPEN_SOURCE 500

#include "unxnote/common.h"
#include "unxnote-common/bug.h"

/* Include XCB for window managment */

#include <xcb/xcb.h>

/**
 * Initialize UNXNote Library
 */

void unxnote_init(unsigned int n, ...);

/**
 * Free library data and close X connection
 */

void unxnote_free(void);

/**
 * Display a notification with a custom vendor cookie
 *
 * @param vendor - UNXNoteVendor enumerator
 * @param from - Message address
 * @param msg - Message buffer
 */

void unxnote_open_window(const char *cookie_name, char *from, char *msg);

/**
 * Display UNXNote window manager
 */

void unxnote_update();

#endif // _UNXNOTE_H
