#define _GNU_SOURCE

#include "source.h"
#include "data_structures.h"
#include "params.h"
#include "utils.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

int g_origin;

int g_requests_msq;
int g_city_id;
int g_origin_msq;

void source_handler(int signum);
int generate_valid_pos();
/*
====================================
  PUBLIC
====================================
*/

int create_origin_msq()
{
  int id = msgget(getpid(), 0660 | IPC_CREAT);
  DEBUG_RAISE_INT(getppid(), id);

  g_origin_msq = id;

  return id;
}

void init_data(int requests_msq, int city_id)
{
  g_origin = -1;
  g_requests_msq = requests_msq;
  g_city_id = city_id;
}

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = source_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

int generate_taxi_request(RequestMsg *req)
{
  req->mtype = 1;
  req->mtext.origin = g_origin;
  req->mtext.destination = generate_valid_pos();
  return 0;
}

void send_taxi_request(RequestMsg *req)
{
  int err = msgsnd(g_requests_msq, req, sizeof req->mtext, 0);
  DEBUG_RAISE_INT(getppid(), err);
}

void save_source_position(int origin_msq)
{
  OriginMsg msg;
  int err;

  err = msgrcv(origin_msq, &msg, sizeof msg.mtext, 0, 0);
  DEBUG_RAISE_INT(getppid(), err);

  g_origin = msg.mtext[0];
}

/*
====================================
  PRIVATE
====================================
*/

/* Signal handler del processo source */
void source_handler(int signum)
{
  RequestMsg req;

  switch (signum)
  {
  case SIGTERM:
    msgctl(g_origin_msq, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
    break;

  case SIGUSR1:
    generate_taxi_request(&req);
    send_taxi_request(&req);
    break;

  case SIGUSR2:
    msgctl(g_origin_msq, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
    break;

  default:
    break;
  }
}

/* Genera una posizione valida per la destinazione */
int generate_valid_pos()
{
  City city = shmat(g_city_id, NULL, SHM_RDONLY);
  int pos = -1, done = FALSE;
  while (!done)
  {
    pos = rand_int(0, SO_HEIGHT * SO_WIDTH - 1);

    if (city[pos].type != CELL_HOLE)
    {
      done = TRUE;
    }
  }

  shmdt(city);
  return pos;
}
