#define _GNU_SOURCE

#include "taxigen_lib.h"
#include "executables.h"
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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TAXIPIDS_SIZE (SO_TAXI + 1)

/* ENTITÀ */
pid_t *g_taxi_pids; /* pid dei taxi in vita */
int g_index;        /* indice di g_taxi_pids */

void taxigen_handler(int signum);
int find_pid_index(pid_t pid, pid_t arr[]);
void prepare_taxi_args(char *args[], int pos, int is_respawned);
void send_signal_to_taxis(int signal);

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
  DEBUG_RAISE_ADDR(g_taxi_pids);
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

pid_t create_taxi(int pos, int is_respawned)
{
  char *args[5] = {TAXI_OBJ, NULL, NULL, NULL, NULL};
  pid_t pid = fork();
  int err;

  DEBUG_RAISE_INT(getpid(), pid);

  if (pid == 0)
  {
    prepare_taxi_args(args, pos, is_respawned);
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      kill(getppid(), SIGTERM);
      exit(EXIT_FAILURE);
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
  int i, status, child_pid;

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
    send_signal_to_taxis(SIGTERM);

    while ((child_pid = wait(&status)) != -1)
    {
      /* printf("taxi %d exited with status %d\n", child_pid, WEXITSTATUS(status)); */
    }

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
  for (i = 0; i < TAXIPIDS_SIZE; i++)
  {
    if (g_taxi_pids[i] != 0)
    {
      kill(g_taxi_pids[i], signal);
    }
  }
}

/* Prepare gli args del processo taxi */
void prepare_taxi_args(char *args[], int pos, int is_respawned)
{
  args[1] = malloc(sizeof(char) * 12);
  DEBUG_RAISE_ADDR(args[1]);
  sprintf(args[1], "%d", is_respawned);

  args[2] = malloc(sizeof(char) * 12);
  DEBUG_RAISE_ADDR(args[2]);
  sprintf(args[2], "%d", getppid());

  args[3] = malloc(sizeof(char) * 12);
  DEBUG_RAISE_ADDR(args[3]);
  sprintf(args[3], "%d", pos);
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