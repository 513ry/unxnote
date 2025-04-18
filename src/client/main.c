/** main.c
 * Copyright (c) 2025, Daniel Sierpiński All rights reserved.
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
 * Simple C client example for UNXNote notification system
 */

/* It's recommended to use POSIX SUSv2+ source for UNXNote client */

#define _XOPEN_SOURCE 500
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "unxnote-common/pshm_data.h"
#include "unxnote-common/bug.h"
#if defined(UNX_NO_STDIO)
static void usage(const char *name) {}
#else
#include <stdio.h>

static void
usage(const char *name)
{
  static const char *const usage_msg[] = {
    "commands:",
    "exit                         exit server",
    NULL
  };
  const char *const *p = usage_msg;

  printf("Usage: %s [--] msg [cookie] [from]\n" \
	 "Usage: %s command [arg] ...\n", name, name);
  while (*p)
    printf("  %s\n", *p++);
}
#endif

int
main(int argc, char **argv)
{
  size_t                 len;        // Message length
  int                    shmdes;     // Shared memory object descriptor
  struct posix_shmbuf   *p_shmbuf;   // POSIX virtual memory string buffer

  /* Too little arguments */

  if (argc < 2) {
    usage(argv[0]);
    return EINVAL;
  }

  /* Too many arguments */

  if (argc > 4) {
    usage(argv[0]);
    return E2BIG;
  }

  /* Prepend shell arguments to the parser buffer */

  len = strlen(argv[1]) + 1;
  if (argc > 2) len += (strlen(argv[2]) + 2);
  if (argc > 3) len += (strlen(argv[3]) + 2);
  if (len > MAX_BUF_SIZE)
    unxnote_bug("no buffer space", ENOBUFS);

  char buffer[len];

  unxnote_log("len: %i\n", len);

  if (argc == 2)
    snprintf(buffer, len, "%s", argv[1]);
  else if (argc == 3)
    snprintf(buffer, len, "%s %s", argv[1], argv[2]);
  else if (argc == 4)
    snprintf(buffer, len, "%s %s %s", argv[1], argv[2], argv[3]);

  /* Open the standard POSIX shared memory read-only descriptor */

  shmdes = shm_open(SHM_NAME, O_RDWR, 0);
  if (shmdes == -1)
    unxnote_bug("shm_open", errno);

  p_shmbuf = posix_shmmap(shmdes, len + sizeof(struct posix_shmbuf),
			  PROT_READ | PROT_WRITE);

  /* Check if server semaphore is open and close if it's closed (S2) */

  if (sem_trywait(&p_shmbuf->sem2) == -1)
    unxnote_bug("sem2 is closed or busy", EXIT_FAILURE);

  /* Copy data into shared memory object */

  p_shmbuf->count = len;

  unxnote_log("message: %s\n", buffer);
  memcpy(&p_shmbuf->buf, buffer, len);

  /* Tell peer that it can now access shared memory (S1) */

  if (sem_post(&p_shmbuf->sem1) == -1)
    unxnote_bug("sem_post", errno);

  exit(EXIT_SUCCESS);
}
