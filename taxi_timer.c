#include "params.h"
#include "utils.h"
#include "data_structures.h"
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char const *argv[]) {
  int err;
  int taxi_pid = getppid();
  while (TRUE) {
    sleep_for(SO_TIMEOUT, 0);

    if (errno == 0) {
      err = kill(taxi_pid, SIGUSR1);
      DEBUG_RAISE_INT(getpid(), err);
      exit(0);
    } else {
      errno = 0;
    }
  }
  return 0;
}
