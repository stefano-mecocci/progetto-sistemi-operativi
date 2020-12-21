#define _GNU_SOURCE

#include "master.h"
#include "params.h"
#include <stdio.h>
#include "data_structures.h"
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <signal.h>

int g_city_id;

void check_params() {
  if (SO_SOURCES > (SO_WIDTH * SO_HEIGHT - SO_HOLES)) {
    printf("ERRORE: troppe sorgenti, città troppo piccola o troppe buche\n");
    exit(EXIT_FAILURE);
  }

  if (SO_CAP_MIN > SO_CAP_MAX) {
    printf("ERRORE: SO_CAP_MIN maggiore di SO_CAP_MAX\n");
    exit(EXIT_FAILURE);
  }

  if (SO_TIMENSEC_MIN > SO_TIMENSEC_MAX) {
    printf("ERRORE: SO_TIMENSEC_MIN maggiore di SO_TIMENSEC_MAX\n");
    exit(EXIT_FAILURE);
  }
}

int create_city() {
  int size = sizeof(Cell) *  SO_WIDTH * SO_HEIGHT;
  int id = shmget(getpid(), size, 0660 | IPC_CREAT);
  g_city_id = id;

  if (id == -1) {
    printf("ERRNO: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  return id;
}

void clear_ipc_memory() {
  int err = shmctl(g_city_id, IPC_RMID, NULL);

  if (err < 0) {
    printf("ERRNO: %d\n", errno);
    exit(EXIT_FAILURE); 
  }
}

/* PRIVATE */
int rand_int(int min, int max) {
  if (min == max) {
    return min;
  } else {
    srand(time(NULL));
    return (rand() % (max - min + 1)) + min;
  }
}

/* PRIVATE */
void init_cell(Cell c) {
  c.type = CELL_NORMAL;
  c.capacity = rand_int(SO_CAP_MIN, SO_CAP_MAX);
  c.cross_time = rand_int(SO_TIMENSEC_MIN, SO_TIMENSEC_MAX);
}

void init_city_cells(int city_id) {
  City city = shmat(city_id, NULL, 0);
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++) {
    init_cell(city[i]);
  }

  shmdt(city);
}

/* PRIVATE */
void master_handler(int signum) {
  if (signum == SIGINT) {
    clear_ipc_memory();
    exit(EXIT_FAILURE);
  }
}

void set_handler() {
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = master_handler;

  sigaction(SIGINT, &act, NULL);
}

void print_city(int city_id) {
  City city = shmat(city_id, NULL, 0);
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++) {
    if (i % SO_WIDTH == 0) {
      printf("\n");
    } else {
      printf(". ");
    }
  }

  printf("\n\n");

  shmdt(city);
}

void set_city_holes(int city_id) {
  City city = shmat(city_id, NULL, 0);
  int i = 0, pos = -1;

  while (i < SO_HOLES) {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    /* TODO: finire il riprova per ottenere buche non adiacenti */
  }

  printf("\n\n");

  shmdt(city);
}