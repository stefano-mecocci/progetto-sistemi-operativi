#define _GNU_SOURCE

#include "params.h"
#include "data_structures.h"
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int sleep_for(int secs, int nanosecs) {
  struct timespec t;
  int err;

  t.tv_sec = secs;
  t.tv_nsec = nanosecs;

  err = nanosleep(&t, NULL);

  return err;
}

int g_sig = 0;

void handler(int signum) {
  if (signum == SIGUSR1) {
    g_sig = 1;
  }
}

void set_handler() {
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGALRM, &act, NULL);
  sigaction(SIGCONT, &act, NULL);
}

int main(int argc, char const *argv[]) {
  set_handler();

  while (TRUE) {
    sleep_for(SO_TIMEOUT, 0);

    if (g_sig == 0) {
      kill(getppid(), SIGUSR1);
      exit(0);
    } else {
      errno = 0;
    }
  }
  return 0;
}
