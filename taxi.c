#define _GNU_SOURCE

#include "taxi.h"
#include "data_structures.h"
#include "params.h"
#include "utils.h"

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

int g_taxi_spawn_msq;
int g_taxi_info_msq;
int g_sync_sems;

pid_t g_master_pid, g_timer_pid;
Taxi g_data;
int g_pos;

void taxi_handler(int signum);
void send_spawn_request();
void block_signal(int signum);
void unblock_signal(int signum);
void send_taxi_data();

/*
====================================
  PUBLIC
====================================
*/

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGCONT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems)
{
  g_taxi_spawn_msq = taxi_spawn_msq;
  g_taxi_info_msq = taxi_info_msq;
  g_sync_sems = sync_sems;
}

void init_data(int master_pid, int pos)
{
  g_master_pid = master_pid;
  g_pos = pos;

  g_data.crossed_cells = 1;
  g_data.max_travel_time = 2;
  g_data.requests = 3;
}

void start_timer()
{
  pid_t pid = fork();
  char *args[2] = {"taxi_timer.o", NULL};
  int err;

  DEBUG_RAISE_INT(pid);

  if (pid == 0)
  {
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      kill(getppid(), SIGTERM);
      exit(EXIT_ERROR);
    }
  }
  else
  {
    g_timer_pid = pid;
  }
}

void receive_ride_request(int requests_msq, Request *req)
{
  int err;
  err = msgrcv(requests_msq, req, sizeof req->mtext, getpid(), 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}

/*
====================================
  PRIVATE
====================================
*/

/* Invia i dati del taxi a master */
void send_taxi_data()
{
  TaxiInfo info;
  int err;

  info.mtype = 1;
  info.mtext[0] = g_data.crossed_cells;
  info.mtext[1] = g_data.max_travel_time;
  info.mtext[2] = g_data.requests;

  err = msgsnd(g_taxi_info_msq, &info, sizeof info.mtext, 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}

void block_signal(int signum)
{
  sigset_t mask;
  bzero(&mask, sizeof mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signal(int signum)
{
  sigset_t mask;
  bzero(&mask, sizeof mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void taxi_handler(int signum)
{
  int i, err;
  TaxiStatus status;
  status.pid = getpid();
  status.position = g_pos;

  switch (signum)
  {
  case SIGINT:
    kill(g_timer_pid, SIGSTOP);
    break;

  case SIGCONT:
    kill(g_timer_pid, SIGCONT);
    break;

  case SIGTERM:

    exit(EXIT_ERROR);
    break;

  case SIGUSR2: /* END OF SIMULATION */
    send_taxi_update(g_taxi_info_msq, ABORTED, status);
    err = sem_op(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);
    DEBUG_RAISE_INT(g_master_pid, err);

    exit(EXIT_TIMER);
    break;

  case SIGUSR1: /* TIMEOUT */
    block_signal(SIGUSR2);

    /* send_taxi_data(); */
    send_taxi_update(g_taxi_info_msq, TIMEOUT, status);
    send_spawn_request();
    err = sem_op(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);
    DEBUG_RAISE_INT(g_master_pid, err);

    unblock_signal(SIGUSR2);
    exit(EXIT_TIMER);
    break;
  }
}

/* Invia una richiesta di spawn a taxigen */
void send_spawn_request()
{
  Spawn req;
  int err;

  req.mtype = RESPAWN;
  req.mtext[0] = getpid();
  req.mtext[1] = g_pos;

  err = msgsnd(g_taxi_spawn_msq, &req, sizeof req.mtext, 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}
