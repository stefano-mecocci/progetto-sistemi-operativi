#ifndef _DATA_STRUCTURES_H
#define _DATA_STRUCTURES_H

#include <sys/types.h>

/* classic types which are always useful */

enum Bool
{
  FALSE,
  TRUE
};

typedef struct tuple
{
  int key;
  int value;
} Tuple;


/* Indexes of the semaphore array sync_sems */

/* why the sem? to syncronize taxi spawn at the start */
#define SEM_SYNC_TAXI 0

/* why the sem? to syncronize sources spawn at the start */
#define SEM_SYNC_SOURCES 1

/* For taxi process fork, SPAWN == is one of the first SO_TAXI */

#define SPAWN 2
#define RESPAWN 3

/* Exit for timer by SO_DURATION seconds */

#define EXIT_TIMER 2

enum cell_type
{
  CELL_NORMAL, /* cell with no taxi */
  CELL_HOLE,   /* hole or obstacle */
  CELL_SOURCE  /* a cell which produces taxi requests */
};

/*
Cella
- type: cell_type
- capacity: max number of taxi in the cell at the same time
- cross_time: time to cross the cell
*/
typedef struct cell
{
  enum cell_type type;
  int capacity;
  int cross_time;
} Cell;

/* City = array of length SO_WIDTH * SO_HEIGHT */
typedef Cell *City;

/* Useful to calculate positions on the city */
typedef struct point
{
  int x;
  int y;
} Point;

/* Structs for linked list of TaxiStats */

typedef struct taxi
{
  pid_t pid;
  int crossed_cells;
  long longest_travel_time;
  int requests;
} TaxiStats;

typedef struct node
{
  TaxiStats taxi_stats;
  struct node *next;
} Node;

typedef Node *List;

/* Represents the current status of the taxi */
typedef struct taxi_status
{
  pid_t pid;
  enum Bool available;
  int position;
  long longest_travel_time;
} TaxiStatus;

/* value of mtype in a TaxiActionMsg */
enum TaxiOps
{
  SPAWNED = 1,
  DEQUEUE = 2,
  PICKUP = 3,
  BASICMOV = 4,
  SERVED = 5,
  TIMEOUT = 6,
  ABORTED = 7
};

/*
Used to notify about taxi status changes between <TaxiOps> operations

mtype: action performed - value between TaxiOps
mtext: status update
*/
typedef struct
{
  long mtype;
  TaxiStatus mtext;
} TaxiActionMsg;

/* Define what a ride is: origin to destination */
typedef struct ride
{
  int origin;
  int destination;
} Ride;

enum RequestType
{
  NORMAL = 1,
  FAILED = 2
};

/*
Taxi request message
- mtype is a value between RequestType enum
- mtext see Ride
*/
typedef struct request
{
  long mtype;
  Ride mtext;
} RequestMsg;

/*
Spawn request message for taxigen:
- mtext = pid of dead taxi (default -1)
*/
typedef struct spawn
{
  long mtype;
  int mtext;
} SpawnMsg;

/*
Define different types of printing for print_city function
see lib/utils.c
*/
enum PrintMode
{
  ACT_CAPACITY,
  SOURCES,
  TOP_CELLS
};

#endif