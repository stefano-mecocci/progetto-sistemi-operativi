#define _GNU_SOURCE

#include <sys/sem.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

/* Signal handler del processo taxi */
void taxi_handler(int signum) {
  if (signum == SIGINT) {
    exit(EXIT_FAILURE);
  }
}

void set_handler() {
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_handler;

  sigaction(SIGINT, &act, NULL);
}

void sem_remove(int sem_arr, int sem, int op) {
  struct sembuf sops[1];
  int err;

  sops[0].sem_flg = 0;
  sops[0].sem_num = sem;
  sops[0].sem_op = op;

  err = semop(sem_arr, sops, 1);

  if (err == -1) {
    DEBUG;
    kill(getppid(), SIGINT);
    exit(EXIT_FAILURE);
  }
}

int get_sync_sem() {
  int id = semget(getppid(), 1, 0660);

  if (id == -1) {
    DEBUG;
    kill(getppid(), SIGINT);
    exit(EXIT_FAILURE);
  }

  return id;
}