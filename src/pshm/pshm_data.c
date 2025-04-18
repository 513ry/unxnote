#define _XOPEN_SOURCE 7002
#include "unxnote-common/pshm_data.h"
#include "unxnote-common/bug.h"

#include <sys/stat.h>
#include <unistd.h>

struct posix_shmbuf *
posix_shmmap(int fildes, size_t size, int prot)
{
  struct stat st;

  /* Check previous file length */

  if (fstat(fildes, &st) < 0)
    unxnote_bug("fstat", EXIT_FAILURE);

  size_t len = st.st_size;

  if (size > len) {

    /* Align size to a whole page size */

    size_t ps = sysconf(_SC_PAGESIZE);
    size_t ns = (size + ps - 1) & ~(ps - 1);

    if (ftruncate(fildes, ns) < 0)
      unxnote_bug("ftruncate", EXIT_FAILURE);

#if defined (IBM)

    /* Ensure new memory pages are filled with zeros */

    if (pwrite(fildes, "", 1, ns - 1) < 0)
      unxnote_bug("pwrite");
#endif

    size = ns;
  }

  /*  Map file to memory */

  void *np = mmap(NULL, size, prot, MAP_SHARED, fildes, 0);
  if (np == MAP_FAILED)
    unxnote_bug("mmap", EXIT_FAILURE);

  return (struct posix_shmbuf *)np;
}
