#define _GNU_SOURCE

#include "master.h"
#include "params.h"
#include <stdio.h>
#include "data_structures.h"
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <signal.h>

/* OGGETTI IPC */
int g_city_id;
int g_requests_id;

/* STATISTICHE */
int g_travels;
int * g_sources;
int * g_top_cells;
Taxi g_most_street;
Taxi g_most_long_travel;
Taxi g_most_requests;

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

int create_requests_msq() {
  int id = msgget(getpid(), 0660 | IPC_CREAT);
  g_requests_id = id;

  if (id == -1) {
    printf("ERRNO: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  return id;
}

void clear_ipc_memory() {
  int err = shmctl(g_city_id, IPC_RMID, NULL);
  err += msgctl(g_requests_id, IPC_RMID, NULL);

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
    return (rand() % (max - min + 1)) + min;
  }
}

/* PRIVATE */
void init_cell(Cell c) {
  srand(time(NULL));

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
      if (city[i].type == CELL_HOLE) {
        printf("o ");
      } else {
        printf(". ");
      }
    }
  }

  printf("\n\n");

  shmdt(city);
}

/* PRIVATE */
Point index2point(int index) {
  Point p;

  p.x = index % SO_WIDTH + 1;
  p.y = index / SO_WIDTH + 1;

  return p;
}

/* PRIVATE */
int point2index(Point p) {
  return p.y * SO_WIDTH - 1;
}

/* REF: https://stackoverflow.com/questions/2035522/get-adjacent-elements-in-a-two-dimensional-array */
/* PRIVATE */
void generate_index_list(Point p, int list[]) {
  Point tmp;
  int dx, dy;
  int i = 0;

  for (dx = -1; dx <= 1; dx++) {
    for (dy = -1; dy <= 1; dy++) {
      if (!(dx == 0 && dy == 0)) {
        tmp.x = p.x + dx;
        tmp.y = p.y + dy;

        list[i] = point2index(tmp);
        i++;
      }
    }
  }
}

/* PRIVATE */
int is_valid_hole_point(Point p, City city) {
  int list[8]; /* TODO: sostituire magic constant */
  int i, is_valid = 1;

  generate_index_list(p, list);

  for (i = 0; i < 8; i++) {
    is_valid = is_valid && city[list[i]].type != CELL_HOLE;
  }

  return is_valid;
}

/* PRIVATE */
void place_hole(int pos, City city) {
  city[pos].capacity = 0;
  city[pos].cross_time = 0;
  city[pos].type = CELL_HOLE;
}

void set_city_holes(int city_id) {
  City city = shmat(city_id, NULL, 0);
  int i = 0, pos = -1;

  srand(time(NULL));

  while (i < SO_HOLES) {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    if (is_valid_hole_point(index2point(pos), city)) {
      place_hole(pos, city);
      i++;
    }
  }

  shmdt(city);
}

void init_stats() {
  g_sources = malloc(sizeof(int) * SO_SOURCES);
  g_top_cells = malloc(sizeof(int) * SO_TOP_CELLS);
  g_travels = 0;

  bzero(&g_most_long_travel, sizeof g_most_long_travel);
  bzero(&g_most_requests, sizeof g_most_requests);
  bzero(&g_most_street, sizeof g_most_street);
}