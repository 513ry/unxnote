#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unxnote.h"

int main(int argc, char **argv) {
  unxnote_init(0);
  unxnote_msg("default", "test@mango", "Hello, UNXNote!");
  unxnote_free();
  
  return EXIT_SUCCESS;
}
