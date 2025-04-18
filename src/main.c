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
 * POSIX and XCB notification system
 */

#include "unxnote.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <signal.h>

#include "unxnote-common/pshm_data.h"

#if !defined (UNX_NO_STDIO)
# include <stdio.h>
#endif

#define CLEAN_FNS 2
#define FRAME_TIME_NS 200000000L

static long current_time_ns();
static void free_shm(void);
static void interrupt();

/* Destructors Table (TODO: Overwrite malloc to count allocations) */

typedef void (*std_fn)(void);
static uint32_t cleanups = 0;
static std_fn destructors[CLEAN_FNS] =
  { free_shm, unxnote_free };
static void
clean(void) {
  for (int i = 0; i < cleanups; ++i)
    destructors[i]();
}

int
main(int argc, char **argv)
{
  int                   shmdes;
  struct posix_shmbuf  *p_shmbuf;
  bool                  done;

  signal(SIGINT, interrupt);

  /* Attach destructor function */

  if (atexit(clean) != 0)
    unxnote_bug("atexit", EXIT_FAILURE);

  /* Create shared memory object */

  shmdes = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
  if (shmdes == -1)
    unxnote_bug("shm_open", errno);
  ++cleanups;

  /* Map the object into caller's address spece */

  p_shmbuf = posix_shmmap(shmdes, MAX_BUF_SIZE, PROT_READ | PROT_WRITE);

  /* Initialize semaphores */

  if (sem_init(&p_shmbuf->sem1, 1, 0) == -1)
    unxnote_bug("sem_init-sem1", errno);
  if(sem_init(&p_shmbuf->sem2, 1, 0) == -1)
    unxnote_bug("sem_init-sem2", errno);

  /* Initialize UNXNote internal cookies */

  unxnote_init(0);
  ++cleanups;

  done = false;
  long next_frame_time = current_time_ns();

  /* Open server semaphore (S2) */

  sem_post(&p_shmbuf->sem2);

#if !defined (UNX_NO_STDIO)
  fputs("UNXNote start ...\n", stderr);
#endif

  while (!done) {
    struct timespec ts;
    next_frame_time += FRAME_TIME_NS;

    /* Wait for client semaphore to be posted (S1) */

    if (sem_trywait(&p_shmbuf->sem1) == 0) {
      if (p_shmbuf->buf[0]) {
        unxnote_log("Recieved message: '%s'\n", p_shmbuf->buf);
        if (strcmp(p_shmbuf->buf, "exit")) {
          unxnote_open_window("default", "test@mango", p_shmbuf->buf);
        } else {
          done = true;
        }
        memset(p_shmbuf->buf, 0, sizeof(p_shmbuf->buf));

	/* Tell peer the buffer is accessible to write (S2) */

	sem_post(&p_shmbuf->sem2);
      }
    } else if (!(errno & EAGAIN))
      unxnote_bug("sem_trywait-sem1", errno);

    /* Block event loop with time-out */

    unxnote_update();

    /* Slow frequency of reading messages */

    long now = current_time_ns();
    long sleep_time = next_frame_time - now;

    if (sleep_time > 0) {
      ts.tv_sec = sleep_time / 1000000000L;
      ts.tv_nsec = sleep_time % 1000000000L;

      nanosleep(&ts, NULL);
    }
  }

  if (sem_destroy(&p_shmbuf->sem1) == -1)
    unxnote_bug("sem_destroy-sem1", errno);

  if (sem_destroy(&p_shmbuf->sem2) == -1)
    unxnote_bug("sem_destroy-sem2", errno);

  exit(EXIT_SUCCESS);
}

// --- Static Definitions

static long
current_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

static void
free_shm(void)
{
  shm_unlink(SHM_NAME);
}

static void
interrupt()
{
#if !defined(NO_STDIO)
  putc('\n', stdout);
#endif
  exit(EXIT_SUCCESS);
}
