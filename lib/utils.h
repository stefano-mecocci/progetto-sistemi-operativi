#ifndef _UTILS_H
#define _UTILS_H

#include "data_structures.h"
#include <math.h>
#include <stdio.h>

#define HIT printf("hit\n");

#define DEBUG \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

#define DEBUG_RAISE_INT2(pid, err) \
  if (err < 0)                     \
  {                                \
    DEBUG;                         \
    kill(pid, SIGTERM);            \
    raise(SIGTERM);                \
  }

#define DEBUG_RAISE_INT(err) \
  if (err < 0)                \
  {                           \
    DEBUG;                    \
    raise(SIGTERM);           \
  }
#define DEBUG_RAISE_ADDR(addr) \
  if (addr == NULL)            \
  {                            \
    DEBUG;                     \
    raise(SIGTERM);            \
  }

#define CHECK_FILE(file, name)                \
  if (file == NULL)                           \
  {                                           \
    printf("Error opening file %s!\n", name); \
    exit(EXIT_FAILURE);                                  \
  }

#define NONE_SYMBOL " ."
#define HOLE_SYMBOL " \033[0;31mX\033[0m"
#define SOURCE_SYMBOL " \033[0;33mS\033[0m"

#define IPC_CITY_ID_FILE "./ipc_res/city_id.txt"
#define IPC_SYNC_SEMS_FILE "./ipc_res/sync_sems.txt"
#define IPC_CITY_SEMS_CAP_FILE "./ipc_res/city_sems_cap.txt"
#define IPC_REQUESTS_MSQ_FILE "./ipc_res/requests_msq.txt"
#define IPC_TAXI_SPAWN_MSQ_FILE "./ipc_res/taxi_spawn_msq.txt"
#define IPC_TAXI_INFO_MSQ_FILE "./ipc_res/taxi_info_msq.txt"

/* Returns map point from given index */
extern Point index2point(int index);

/* Conversion from (x, y) to index */
extern int coordinates2index(int x, int y);

/* Returns map index from given point */
extern int point2index(Point p);

/* Returns taxicab distance between map points */
extern int coordinates_delta(int x0, int y0, int x1, int y1);

/* Returns taxicab distance between map points */
extern int points_delta(Point, Point);

/* Returns IPC source id stored in given filename */
extern int read_id_from_file(char *filename);

/* Stores IPC source id in given filename */
extern void write_id_to_file(int id, char *filename);

/* wrapper for nanosleep */
extern int sleep_for(int secs, int nanosecs);

/* semop wrapper to operate on single semaphore */
extern int sem_op(int sem_arr, int sem, int value, short flag);

/* Generate random int between [min, max], if equals returns min */
extern int rand_int(int min, int max);

/* Enqueues taxi status update */
extern int send_taxi_update(int queue_id, enum TaxiOps op, TaxiStatus status);

extern void reset_stopwatch();

extern long record_stopwatch();

/*
Print the city map in ASCII where:
- empty cells (no taxi) are "."
- cells with 1+ taxi are numbers "<n>"
- holes are "x"
*/
extern void print_city(FILE *fd, int city_id, int city_sems_cap, enum PrintMode mode, int (*get_cell_val)(int));

#endif