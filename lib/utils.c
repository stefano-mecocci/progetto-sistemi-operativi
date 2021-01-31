#define _GNU_SOURCE

#include "utils.h"
#include "data_structures.h"
#include "params.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <string.h>

long g_start;

int coordinates2index(int x, int y)
{
  Point p;
  p.x = x;
  p.y = y;
  return point2index(p);
}

/* Conversione indice -> punto */
Point index2point(int index)
{
  Point p;

  p.x = index % SO_WIDTH;
  p.y = floor((float)index / (float)SO_WIDTH);

  return p;
}

/* Conversione punto -> indice */
int point2index(Point p)
{
  if (p.x < 0 || p.x >= SO_WIDTH || p.y < 0 || p.y >= SO_HEIGHT)
  {
    return -1;
  }
  return (SO_WIDTH * p.y) + p.x;
}

/* Returns taxicab distance between map indexes */
int indexes_delta(int idx1, int idx2)
{
  return points_delta(index2point(idx1), index2point(idx2));
}

/* Returns taxicab distance between map points */
int points_delta(Point pt1, Point pt2)
{
  return abs(pt1.x - pt2.x) + abs(pt1.y - pt2.y);
}

/* Returns IPC source id stored in given filename */
int read_id_from_file(char *filename)
{
  FILE *f = fopen(filename, "r");
  int id;

  fscanf(f, "%d", &id);
  fclose(f);

  return id;
}

/* Stores IPC source id in given filename */
void write_id_to_file(int id, char *filename)
{
  FILE *file = fopen(filename, "w+");

  fprintf(file, "%d", id);
  fclose(file);
}

int sleep_for(int secs, int nanosecs)
{
  struct timespec t;
  int err;

  t.tv_sec = secs;
  t.tv_nsec = nanosecs;

  err = nanosleep(&t, NULL);

  return err;
}

/* Un wrapper di semop - Proporre di rimuoverlo e sostituirlo con macro per gain access/release */
int sem_op(int sem_arr, int sem, int value, short flag)
{
  struct sembuf op[1];
  int err;

  op[0].sem_flg = flag;
  op[0].sem_num = sem;
  op[0].sem_op = value;

  err = semop(sem_arr, op, 1);

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

int send_taxi_update(int queue_id, enum TaxiOps op, TaxiStatus status)
{
  TaxiActionMsg msg;
  int err;

  msg.mtype = op;
  msg.mtext = status;
  return msgsnd(queue_id, &msg, sizeof(TaxiStatus), 0);
}

void block_signal(int signum)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signal(int signum)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

long get_milliseconds()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  long millisecondsSinceEpoch =
      (long)(tv.tv_sec) * 1000 +
      (long)(tv.tv_usec) / 1000;
}

void reset_stopwatch()
{
  g_start = get_milliseconds();
}

long record_stopwatch()
{
  return get_milliseconds() - g_start;
}

void print_city(FILE *fd, int city_id, int city_sems_cap, enum PrintMode mode, int (*get_cell_val)(int))
{
  City city = shmat(city_id, NULL, 0);
  int i, taxi_num, val;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    if (i % SO_WIDTH == 0)
    {
      fprintf(fd, "\n");
    }

    if (city[i].type == CELL_HOLE)
    {
      fprintf(fd, HOLE_SYMBOL);
    }
    else if (mode == TOP_CELLS)
    {
      val = get_cell_val(i);
      if (val >= 0)
      {
        fprintf(fd, "%2d", val);
      }
      else
      {
        fprintf(fd, NONE_SYMBOL);
      }
    }
    else if (mode == SOURCES)
    {
      if (city[i].type == CELL_SOURCE)
      {
        fprintf(fd, SOURCE_SYMBOL);
      }
      else
      {
        fprintf(fd, NONE_SYMBOL);
      }
    }
    else
    {
      taxi_num = city[i].capacity - semctl(city_sems_cap, i, GETVAL);

      if (taxi_num == 0)
      {
        fprintf(fd, NONE_SYMBOL);
      }
      else
      {
        fprintf(fd, "%2d", taxi_num);
      }
    }
  }

  fprintf(fd, "\n\n");
  shmdt(city);
}