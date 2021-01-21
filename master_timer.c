#define _GNU_SOURCE

#include "params.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>

int sleep_for(int secs, int nanosecs) {
  struct timespec t;
  int err;

  t.tv_sec = secs;
  t.tv_nsec = nanosecs;

  err = nanosleep(&t, NULL);

  return err;
}

int main(int argc, char const *argv[]) {
  sleep_for(0, 100000000); /* fix temporaneo */

  sleep_for(SO_DURATION, 0);
  kill(getppid(), SIGUSR2);

  return 0;
}
