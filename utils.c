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

/* Conversione indice -> punto */
Point index2point(int index)
{
  Point p; 

  p.x = index % SO_WIDTH + 1;
  p.y = floor((float)index / (float)SO_WIDTH) + 1;

  return p;
}

/* Conversione punto -> indice */
int point2index(Point p) { return (SO_WIDTH * (p.y - 1)) + p.x -1; }

/* Returns taxicab distance between map indexes */
int indexes_delta(int idx1, int idx2)
{
  return points_delta(index2point(idx1), index2point(idx2));
}

int coordinates2index(int x, int y){
  Point p;
  p.x = x;
  p.y = y;
  return point2index(p);
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
  FILE *file = fopen(filename, "w");

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
  /* bzero(&mask, sizeof mask); */
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signal(int signum)
{
  sigset_t mask;
  /* bzero(&mask, sizeof mask); */
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

long get_milliseconds() {
  struct timeval tv;

  gettimeofday(&tv, NULL);

  long millisecondsSinceEpoch =
      (long)(tv.tv_sec) * 1000 +
      (long)(tv.tv_usec) / 1000;
}

void reset_stopwatch() {
  g_start = get_milliseconds();
}

long record_stopwatch() {
  return get_milliseconds() - g_start;
}