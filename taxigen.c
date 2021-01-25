#define _GNU_SOURCE

#include "taxigen.h"
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

/* ENTITÀ */
pid_t *g_taxi_pids; /* pid dei taxi in vita */
int g_index;        /* indice di g_taxi_pids */

void taxigen_handler(int signum);
int find_pid_index(pid_t pid, pid_t arr[]);
int generate_valid_taxi_pos(City city);
void prepare_taxi_args(char *args[], int pos, int isNew);

/*
====================================
  PUBLIC
====================================
*/

int x = 0;

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxigen_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGCONT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_data()
{
  int i;

  g_taxi_pids = malloc(sizeof(pid_t) * TAXIPIDS_SIZE);
  for (i = 0; i < TAXIPIDS_SIZE; i++)
  {
    g_taxi_pids[i] = 0;
  }

  g_index = 0;
}

void receive_spawn_request(int taxi_spawn_msq, SpawnMsg *req)
{
  int err;
  err = msgrcv(taxi_spawn_msq, req, sizeof req->mtext, 0, 0);
  DEBUG_RAISE_INT(getppid(), err);
}

void remove_old_taxi(int city_sems_cap, int pos)
{
  int err;
  err = sem_op(city_sems_cap, pos, 1, 0);
  DEBUG_RAISE_INT(getppid(), err);
}

int set_taxi(int city_id, int city_sems_cap)
{
  City city = shmat(city_id, NULL, 0);
  int pos;

  while (TRUE)
  {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    if (city[pos].type != CELL_HOLE &&
        (sem_op(city_sems_cap, pos, -1, IPC_NOWAIT) == 0))
    {
      break;
    }
  }

  shmdt(city);
  return pos;
}

pid_t create_taxi(int pos, int isNew)
{
  char *args[5] = {"taxi.o", NULL, NULL, NULL, NULL};
  pid_t pid = fork();
  int err;

  DEBUG_RAISE_INT(getppid(), pid);

  if (pid == 0)
  {
    prepare_taxi_args(args, pos, isNew);
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
    return pid;
  }
}

void add_taxi_pid(pid_t pid)
{
  g_taxi_pids[g_index] = pid;
  g_index += 1;
}

void replace_taxi_pid(pid_t old_pid, pid_t new_pid)
{
  int index = find_pid_index(old_pid, g_taxi_pids);

  g_taxi_pids[index] = new_pid;
}


/*
====================================
  PRIVATE
====================================
*/

/* Signal handler di taxigen */
void taxigen_handler(int signum)
{
  int i;

  switch (signum)
  {
  case SIGINT: /* Stop sub processes */
    send_signal_to_taxis(SIGINT);
    raise(SIGSTOP);
    break;
  case SIGCONT: /* Resume sub processes */
    send_signal_to_taxis(SIGCONT);

    break;
  case SIGUSR2:
    send_signal_to_taxis(SIGUSR2);

    exit(EXIT_SUCCESS);
    break;
  case SIGTERM:
    send_signal_to_taxis(SIGTERM);

    exit(EXIT_FAILURE);
    break;
  default:
    break;
  }
}

void send_signal_to_taxis(int signal)
{
  int i;
    for (i = 0; g_taxi_pids[i] != 0; i++)
    {
      kill(g_taxi_pids[i], signal);
    }
}

/* Prepare gli args del processo taxi */
void prepare_taxi_args(char *args[], int pos, int isNew)
{
  args[1] = malloc(sizeof(char) * 12);
  sprintf(args[1], "%d", isNew);

  args[2] = malloc(sizeof(char) * 12);
  sprintf(args[2], "%d", getppid());

  args[3] = malloc(sizeof(char) * 12);
  sprintf(args[3], "%d", pos);
}

/* Genera una posizione valida per un taxi */
int generate_valid_taxi_pos(City city)
{
  int pos = -1, done = FALSE;

  while (!done)
  {
    pos = rand_int(0, SO_HEIGHT * SO_WIDTH - 1);

    if (city[pos].type != CELL_HOLE && city[pos].act_capacity > 0)
    {
      done = TRUE;
    }
  }

  return pos;
}

/* Trova l'indice di un pid salvato in array */
int find_pid_index(pid_t pid, pid_t arr[])
{
  int i, done = FALSE;
  int result = -1;

  for (i = 0; g_taxi_pids[i] != 0 && !done; i++)
  {
    if (arr[i] == pid)
    {
      result = i;
      done = TRUE;
    }
  }

  return result;
}