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

#include "unxnote/common.h"
#include "unxnote/cookie.h"

#define UNXNOTE_FONT "../ttf/VictorMonoNerdFont-Medium.ttf"

/*
 * Initialize UNXNote Library
 *
 * @return true on success and false if window fails to initialize
 */
bool unxnote_init(unsigned int n, ...);

/*
 * Free library data and close X connection
 *
 * @return true on success and false if not initialized
 */
bool unxnote_free(void);

/*
 * Display a notification with a custom vendor cookie
 *
 * @param vendor - UNXNoteVendor enumerator
 * @param from - Message address
 * @param msg - Message buffer
 * @return true on success and false on any error
 */
bool unxnote_msg(char *cookie, char *from, char *msg);

/*
 * Display a primitive notification without cookie
 *
 * @param glyph - UTF8 glyph
 * @param title - Title bar string
 * @param msg
 * @param fg_color
 * @param bg_color
 * @param expire - Time in ms before notification expire. 0 for none
 * @raturn true on success and false on any error
 */
bool unxnote_generic(char glyph, char *title, char *msg, uint16_t fg_color,
		     uint16_t bg_color, uint16_t expire);

#endif // _UNXNOTE_H
