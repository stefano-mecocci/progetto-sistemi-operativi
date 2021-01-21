#define _GNU_SOURCE

#include "taxi.h"
#include "data_structures.h"
#include "params.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TAXIPIDS_SIZE (SO_TAXI + 1)
#define DEBUG                                                                  \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

#define DEBUG_RAISE_INT(err)                                                   \
  if (err < 0) {                                                               \
    DEBUG;                                                                     \
    kill(g_master_pid, SIGINT);                                                   \
    raise(SIGINT);                                                             \
  }

int g_taxi_spawn_msq;
int g_taxi_info_msq;
int g_sync_sems;

pid_t g_master_pid;
Taxi g_data;
int g_pos;

void taxi_handler(int signum);
int sem_op(int sem_arr, int sem, int value, short flag);
void send_spawn_request();
void block_signal(int signum);
void unblock_signal(int signum);
void send_taxi_data();

/*
====================================
  PUBLIC
====================================
*/

int read_id_from_file(char *filename) {
  FILE *f = fopen(filename, "r");
  int id;

  fscanf(f, "%d", &id);
  fclose(f);

  return id;
}

void set_handler() {
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems) {
  g_taxi_spawn_msq = taxi_spawn_msq;
  g_taxi_info_msq = taxi_info_msq;
  g_sync_sems = sync_sems;
}

void init_data(int master_pid, int pos) {
  g_master_pid = master_pid;
  g_pos = pos;

  g_data.crossed_cells = 1;
  g_data.max_travel_time = 2;
  g_data.requests = 3;
}

int sem_decrease(int sem_arr, int sem, int value, short flag) {
  return sem_op(sem_arr, sem, value, flag);
}

pid_t start_timer() {
  pid_t pid = fork();
  char *args[2] = {"taxi_timer", NULL};
  int err;

  DEBUG_RAISE_INT(pid);

  if (pid == 0) {
    err = execve(args[0], args, environ);

    if (err == -1) {
      DEBUG;
      kill(getppid(), SIGINT);
      exit(EXIT_ERROR);
    }
  } else {
    return pid;
  }
}

/*
====================================
  PRIVATE
====================================
*/

/* Invia i dati del taxi a master */
void send_taxi_data() {
  TaxiInfo info;
  int err;

  info.mtype = 1;
  info.mtext[0] = g_data.crossed_cells;
  info.mtext[1] = g_data.max_travel_time;
  info.mtext[2] = g_data.requests;

  err = msgsnd(g_taxi_info_msq, &info, sizeof info.mtext, 0);
  DEBUG_RAISE_INT(err);
}

void block_signal(int signum) {
  sigset_t mask;
  bzero(&mask, sizeof mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signal(int signum) {
  sigset_t mask;
  bzero(&mask, sizeof mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void taxi_handler(int signum) {
  int i;

  if (signum == SIGINT) {
    exit(EXIT_ERROR);
  } else if (signum == SIGUSR2) {
    send_taxi_data();
    sem_decrease(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);

    exit(EXIT_TIMER);
  } else if (signum == SIGUSR1) {
    block_signal(SIGUSR2);

    send_taxi_data();
    send_spawn_request();
    sem_decrease(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);

    unblock_signal(SIGUSR2);
    exit(EXIT_TIMER);
  }
}

/* Invia una richiesta di spawn a taxigen */
void send_spawn_request() {
  Spawn req;
  int err;

  req.mtype = RESPAWN;
  req.mtext[0] = getpid();
  req.mtext[1] = g_pos;

  err = msgsnd(g_taxi_spawn_msq, &req, sizeof req.mtext, 0);
  DEBUG_RAISE_INT(err);
}

/* Un wrapper di semop */
int sem_op(int sem_arr, int sem, int value, short flag) {
  struct sembuf op[1];
  int err;

  op[0].sem_flg = flag;
  op[0].sem_num = sem;
  op[0].sem_op = value;

  err = semop(sem_arr, op, 1);
  DEBUG_RAISE_INT(err);

  return err;
}
