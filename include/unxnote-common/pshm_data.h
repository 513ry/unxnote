#ifndef _PSHM_DATA_H
#define _PSHM_DATA_H

#include <semaphore.h>
#include <sys/mman.h>

#define MAX_BUF_SIZE 256

/* Virtual file system address of the shared memory */

#define SHM_NAME "/unxnote"

struct posix_shmbuf {
  sem_t  sem1;           // Daemon set semaphore
  sem_t  sem2;           // Client set semaphore
  size_t count;          // Number of bytes used in buffer
  char   buf[MAX_BUF_SIZE]; // Data being transferre
};

// --- API

struct posix_shmbuf *posix_shmmap(int fildes, size_t size, int prot);

#endif // _PSHM_DATA_H
