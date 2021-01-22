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

#define DEBUG \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

#define DEBUG_RAISE_INT(err) \
  if (err < 0)               \
  {                          \
    DEBUG;                   \
    kill(getppid(), SIGINT); \
    raise(SIGINT);           \
  }

int g_origin;

int g_requests_msq;
int g_city_id;
int g_origin_msq;

void source_handler(int signum);
int generate_valid_pos();
int rand_int(int min, int max);
int sem_op(int sem_arr, int sem, int value, short flag);

/*
====================================
  PUBLIC
====================================
*/

int create_origin_msq()
{
  int id = msgget(getpid(), 0660 | IPC_CREAT);
  g_origin_msq = id;

  DEBUG_RAISE_INT(id);

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
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

int sem_decrease(int sem_arr, int sem, int value, short flag)
{
  return sem_op(sem_arr, sem, value, flag);
}

void generate_taxi_request(Request *req)
{
  printf("generating request\n");
  /*req->mtype = 1;  replace with find_nearest_taxi_pid() */
  int err = find_nearest_taxi_pid();
  DEBUG_RAISE_INT(err);
  req->mtype = err;
  printf("Found taxi with pid=%d:", req->mtype);
  req->mtext[0] = g_origin;
  req->mtext[1] = generate_valid_pos();
}

void send_taxi_request(Request *req)
{
  int err = msgsnd(g_requests_msq, req, sizeof req->mtext, 0);
  DEBUG_RAISE_INT(err);
}

void save_source_position(int origin_msq)
{
  Origin msg;
  int err;

  err = msgrcv(origin_msq, &msg, sizeof msg.mtext, 0, 0);
  DEBUG_RAISE_INT(err);

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
  case SIGINT:
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

int sem_op(int sem_arr, int sem, int value, short flag)
{
  struct sembuf sops[1];
  int err;

  sops[0].sem_flg = flag;
  sops[0].sem_num = sem;
  sops[0].sem_op = value;

  err = semop(sem_arr, sops, 1);
  DEBUG_RAISE_INT(err);

  return err;
}

/* Genera un numero random fra [min, max], se min == max ritorna min */
int rand_int(int min, int max)
{
  if (min == max)
  {
    return min;
  }
  else
  {
    return (rand() % (max - min + 1)) + min;
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
  int i = 0;
  pid_t pid = 0;
  TaxiStatus taxi;
  while (i < SO_TAXI && min_distance != 0)
  {
    taxi = g_taxi_positions[i];
    
    if(taxi.available == TRUE && (taxi.position - g_origin) < min_distance){
      /* use (dx + dy) to calc taxicab distance - we already have a decent approximation
      instead of using a* (more precise -> heavier)  */      
      min_distance = indexes_delta(taxi.position, g_origin);
      pid = taxi.pid;
    }

    i++;
  }
  
  if(pid == 0){
    /* raise error - no available taxi found */
    return -1;
  }

  return pid;
}
