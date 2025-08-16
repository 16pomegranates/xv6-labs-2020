#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // require exactly one argument: ticks
  if (argc != 2) {
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1);
  }

  int ticks = atoi(argv[1]);  // convert string to int
  if (ticks < 0) {
    fprintf(2, "sleep: ticks must be >= 0\n");
    exit(1);
  }

  sleep(ticks);               // system call: sleep for 'ticks'
  exit(0);                    // terminate program
}
