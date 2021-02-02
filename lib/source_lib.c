#define _GNU_SOURCE

#include "source_lib.h"
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

void source_handler(int signum);
int generate_valid_pos();
/*
====================================
  PUBLIC
====================================
*/

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

  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

int generate_taxi_request(RequestMsg *req)
{
  req->mtype = NORMAL;
  req->mtext.origin = g_origin;
  req->mtext.destination = generate_valid_pos();
  return 0;
}

void send_taxi_request(RequestMsg *req)
{
  int err = msgsnd(g_requests_msq, req, sizeof req->mtext, 0);
  DEBUG_RAISE_INT2(getppid(), err);
}

void set_source_position(int position)
{
  g_origin = position;
}
/*
====================================
  PRIVATE
====================================
*/

/* Signal handler del processo source */
void source_handler(int signum)
{
  switch (signum)
  {
  case SIGTERM:
    exit(EXIT_SUCCESS);
    break;

  case SIGUSR1:
    /* Only used to interrupt sleep in main */
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
