#define _GNU_SOURCE

#include "utils.h"
#include "data_structures.h"
#include "params.h"
#include <stdlib.h>    
#include <stdio.h>


/* Conversione indice -> punto */
Point index2point(int index) {
  Point p;

  p.x = index % SO_WIDTH;
  p.y = index / SO_WIDTH;

  return p;
}

/* Conversione punto -> indice */
int point2index(Point p) { return SO_WIDTH * p.y + p.x; }

/* Returns taxicab distance between map indexes */
int indexes_delta(int idx1, int idx2){
    return points_delta(index2point(idx1), index2point(idx2));
}

/* Returns taxicab distance between map points */
int points_delta(Point pt1, Point pt2){
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
void write_id_to_file(int id, char *filename) {
  FILE *file = fopen(filename, "w");

  fprintf(file, "%d", id);
  fclose(file);
}

int sleep_for(int secs, int nanosecs) {
  struct timespec t;
  int err;

  t.tv_sec = secs;
  t.tv_nsec = nanosecs;

  err = nanosleep(&t, NULL);

  return err;
}