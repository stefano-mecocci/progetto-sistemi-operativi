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
int g_taxi_list_mem_id;

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

void init_data(int requests_msq, int city_id, int taxi_list_mem_id)
{
  g_origin = -1;
  g_requests_msq = requests_msq;
  g_city_id = city_id;
  g_taxi_list_mem_id = taxi_list_mem_id;
}

void set_handler(int g_taxi_list_mem_id)
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = source_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void generate_taxi_request(Request *req)
{
  printf("generating request\n");
  int taxi_pid = find_nearest_taxi_pid();
  DEBUG;
  /* DEBUG_RAISE_INT(getppid(), taxi_pid); */
  req->mtype = taxi_pid;
  printf("Found taxi with pid=%d:", req->mtype);
  req->mtext[0] = g_origin;
  req->mtext[1] = generate_valid_pos();
}

void send_taxi_request(Request *req)
{
  int err = msgsnd(g_requests_msq, req, sizeof req->mtext, 0);
  DEBUG_RAISE_INT(getppid(), err);
}

void save_source_position(int origin_msq)
{
  Origin msg;
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
  Request req;

  switch (signum)
  {
  case SIGTERM:
    msgctl(g_origin_msq, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
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
  City city = shmat(g_city_id, NULL, 0);
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

/* Returns the nearest taxi pid to g_origin - needed for ride request */
pid_t find_nearest_taxi_pid()
{
  int min_distance = SO_WIDTH + SO_HEIGHT;
  int i = 0, err, current_distance;
  pid_t pid = 0;
  TaxiStatus taxi;
  TaxiStatus *taxis = shmat(g_taxi_list_mem_id, NULL, 0);

  while (i < SO_TAXI && min_distance != 0)
  {
    taxi = taxis[i];

    current_distance = indexes_delta(taxi.position, g_origin);
    if (taxi.available == TRUE && current_distance < min_distance)
    {
      /* use (dx + dy) to calc taxicab distance - we already have a decent approximation
      instead of using a* (more precise -> heavier)  */
      min_distance = current_distance;
      pid = taxi.pid;
    }

    i++;
  }

  shmdt(taxis);

  if (pid == 0)
  {
    /* raise error - no available taxi found */
    return -1;
  }

  return pid;
}
