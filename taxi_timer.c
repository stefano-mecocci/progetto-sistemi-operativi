#include "params.h"
#include "data_structures.h"
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char const *argv[]) {
  while (TRUE) {
    sleep_for(SO_TIMEOUT, 0);

    if (errno == 0) {
      kill(getppid(), SIGUSR1);
      exit(0);
    } else {
      errno = 0;
    }
  }
  return 0;
}
