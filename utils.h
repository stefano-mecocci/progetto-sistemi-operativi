#include "data_structures.h"

#define DEBUG \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

/* Returns map point from given index */
extern Point index2point(int index);

/* Returns map index from given point */
extern int point2index(Point p);

/* Returns taxicab distance between map indexes */
extern int indexes_delta(int, int);

/* Returns taxicab distance between map points */
extern int points_delta(Point, Point);

/* Returns IPC source id stored in given filename */
extern int read_id_from_file(char *filename);

/* Stores IPC source id in given filename */
extern  void write_id_to_file(int id, char * filename);

extern int sleep_for(int secs, int nanosecs);