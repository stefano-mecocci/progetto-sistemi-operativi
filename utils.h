#include "data_structures.h"
#include <math.h>

#define HIT printf("hit\n");

#define DEBUG \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

#define GET_MACRO(_1,_2,NAME,...) NAME
#define DEBUG_RAISE_INT(...) GET_MACRO(__VA_ARGS__, DEBUG_RAISE_INT2, DEBUG_RAISE_INT1)(__VA_ARGS__)

#define DEBUG_RAISE_INT2(pid, err) \
  if (err < 0)               \
  {                          \
    DEBUG;                   \
    kill(pid, SIGTERM); \
    raise(SIGTERM);           \
  }

#define DEBUG_RAISE_INT1(err) \
  if (err < 0)               \
  {                          \
    DEBUG;                   \
    raise(SIGTERM);           \
  }
#define DEBUG_RAISE_ADDR(addr) \
  if (addr == NULL)            \
  {                            \
    DEBUG;                     \
    raise(SIGTERM);             \
  }

#define CHECK_FILE(file, name) \
    if (file == NULL)\
    {\
        printf("Error opening file %s!\n", name);\
        exit(1);\
    }\

/* Returns map point from given index */
extern Point index2point(int index);

extern int coordinates2index(int x, int y);

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

extern int sem_op(int sem_arr, int sem, int value, short flag);

/* Genera un numero random fra [min, max], se min == max ritorna min */
extern int rand_int(int min, int max);

/* Enqueues taxi status update */
extern int send_taxi_update(int queue_id, enum TaxiOps op, TaxiStatus status);

extern void block_signal(int signum);

extern void unblock_signal(int signum);

extern long get_milliseconds();

extern void reset_stopwatch();

extern long record_stopwatch();