#define _GNU_SOURCE

#include "params.h"
#include "utils.h"
#include "data_structures.h"
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void await_start();
void run_timer();
void set_handler();
void taxi_timer_handler(int signum);

int main(int argc, char const *argv[]) {
  set_handler();
  await_start();  
}

void await_start()
{
  sigset_t x;
  sigemptyset(&x);
  sigfillset(&x);
  sigdelset(&x, SIGUSR1);
  sigsuspend(&x);
}

void run_timer(){
  int err;
  int taxi_pid = getppid();
  printf("Started timer\n");

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

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_timer_handler;

  sigaction(SIGUSR1, &act, NULL);
}


void taxi_timer_handler(int signum)
{
  int i, err;

  switch (signum)
  {
  case SIGUSR1: 
    run_timer();
    exit(EXIT_SUCCESS);
    break;

  }
}