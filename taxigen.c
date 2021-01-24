#define _GNU_SOURCE

#include "taxigen.h"
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
    kill(getppid(), SIGINT);                                                   \
    raise(SIGINT);                                                             \
  }

/* ENTITÀ */
pid_t *g_taxi_pids; /* pid dei taxi in vita */
int g_index;        /* indice di g_taxi_pids */

void taxigen_handler(int signum);
int find_pid_index(pid_t pid, pid_t arr[]);
int rand_int(int min, int max);
int generate_valid_taxi_pos(City city);
void prepare_taxi_args(char *args[], int pos, int isNew);
int sem_op(int sem_arr, int sem, int value, short flag);

/*
====================================
  PUBLIC
====================================
*/

int x = 0;

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

  act.sa_handler = taxigen_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_data() {
  int i;

  g_taxi_pids = malloc(sizeof(pid_t) * TAXIPIDS_SIZE);
  for (i = 0; i < TAXIPIDS_SIZE; i++) {
    g_taxi_pids[i] = 0;
  }

  g_index = 0;
}

void receive_spawn_request(int taxi_spawn_msq, Spawn *req) {
  int err;
  err = msgrcv(taxi_spawn_msq, req, sizeof req->mtext, 0, 0);
  DEBUG_RAISE_INT(err);
}

int sem_decrease(int sem_arr, int sem, int value, short flag) {
  return sem_op(sem_arr, sem, value, flag);
}

int sem_increase(int sem_arr, int sem, int value, short flag) {
  return sem_op(sem_arr, sem, value, flag);
}

void remove_old_taxi(int city_sems_cap, int pos) {
  sem_increase(city_sems_cap, pos, 1, 0);
}

int set_taxi(int city_id, int city_sems_cap) {
  City city = shmat(city_id, NULL, 0);
  int pos;

  while (TRUE) {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    if (city[pos].type != CELL_HOLE &&
        (sem_decrease(city_sems_cap, pos, -1, IPC_NOWAIT) == 0)) {
      break;
    }
  }

  shmdt(city);
  return pos;
}

pid_t create_taxi(int pos, int isNew) {
  char *args[5] = {"taxi", NULL, NULL, NULL, NULL};
  pid_t pid = fork();
  int err;

  DEBUG_RAISE_INT(pid);

  if (pid == 0) {
    prepare_taxi_args(args, pos, isNew);
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

void add_taxi_pid(pid_t pid) {
  g_taxi_pids[g_index] = pid;
  g_index += 1;
}

void replace_taxi_pid(pid_t old_pid, pid_t new_pid) {
  int index = find_pid_index(old_pid, g_taxi_pids);

  g_taxi_pids[index] = new_pid;
}

/*
====================================
  PRIVATE
====================================
*/

/* Signal handler di taxigen */
void taxigen_handler(int signum) {
  int i;

  switch (signum) {
  case SIGUSR2:
    for (i = 0; g_taxi_pids[i] != 0; i++) {
      kill(g_taxi_pids[i], SIGUSR2);
    }

    exit(EXIT_SUCCESS);
    break;
  case SIGINT:
    for (i = 0; g_taxi_pids[i] != 0; i++) {
      kill(g_taxi_pids[i], SIGINT);
    }

    exit(EXIT_FAILURE);
    break;
  default:
    break;
  }
}

/* Prepare gli args del processo taxi */
void prepare_taxi_args(char *args[], int pos, int isNew) {
  args[1] = malloc(sizeof(char) * 12);
  sprintf(args[1], "%d", isNew);

  args[2] = malloc(sizeof(char) * 12);
  sprintf(args[2], "%d", getppid());

  args[3] = malloc(sizeof(char) * 12);
  sprintf(args[3], "%d", pos);
}

/* Genera una posizione valida per un taxi */
int generate_valid_taxi_pos(City city) {
  int pos = -1, done = FALSE;

  while (!done) {
    pos = rand_int(0, SO_HEIGHT * SO_WIDTH - 1);

    if (city[pos].type != CELL_HOLE && city[pos].act_capacity > 0) {
      done = TRUE;
    }
  }

  return pos;
}

/* Un wrapper di semop */
int sem_op(int sem_arr, int sem, int value, short flag) {
  struct sembuf op[1];
  int err;

  op[0].sem_flg = flag;
  op[0].sem_num = sem;
  op[0].sem_op = value;

  err = semop(sem_arr, op, 1);

  if (errno != EAGAIN) {
    DEBUG_RAISE_INT(err);
  }

  return err;
}

/* Genera un numero random fra [min, max], se min == max ritorna min */
int rand_int(int min, int max) {
  if (min == max) {
    return min;
  } else {
    return (rand() % (max - min + 1)) + min;
  }
}

/* Trova l'indice di un pid salvato in array */
int find_pid_index(pid_t pid, pid_t arr[]) {
  int i, done = FALSE;
  int result = -1;

  for (i = 0; g_taxi_pids[i] != 0 && !done; i++) {
    if (arr[i] == pid) {
      result = i;
      done = TRUE;
    }
  }

  return result;
}