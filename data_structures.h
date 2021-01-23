#ifndef _DATA_STRUCTURES_H
#define _DATA_STRUCTURES_H

#include <sys/types.h>
#include <stdlib.h>

#define SEM_SYNC_TAXI 0
#define SEM_SYNC_SOURCES 1
#define SEM_ALIVES_TAXI 2

/* Tipo di cella */
enum cell_type
{
  CELL_NORMAL,
  CELL_HOLE,
  CELL_SOURCE
};

/*
Cella
- type: normale, buca o di origine
- capacity: max. di taxi che ci possono essere
- cross_time: tempo necessario per attraversare la cella
*/
typedef struct cell
{
  int type;
  int capacity;
  int act_capacity;
  int cross_time;
  int crossing_num;
} Cell;

/* Città = array di lunghezza SO_WIDTH * SO_HEIGHT */
typedef Cell *City;

/* Utile per calcolare posizioni sulla città */
typedef struct point
{
  int x;
  int y;
} Point;

/* Per comodità */
enum Bool
{
  FALSE,
  TRUE
};

/* Struttura abbinata ad un processo taxi */
typedef struct taxi
{
  int crossed_cells;
  int max_travel_time;
  int requests;
} Taxi;

/* Represents the current status of the taxi */
typedef struct taxi_status
{
  pid_t pid;
  enum Bool available;
  int position;
} TaxiStatus;

enum TaxiOps
{
  SPAWNED,
  PICKUP,
  BASICMOV,
  SERVED,
  TIMEOUT,
  ABORTED
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

/*
Messaggio di richiesta taxi:
- mtext == Taxi
*/
typedef struct taxi_info
{
  long mtype;
  int mtext[sizeof(int) * 3];
} TaxiInfo;

#define REQ_SIZE 2

/*
Messaggio di richiesta taxi:
- mtext indica punti di origine e destinazione [origin, destination] 
*/
typedef struct request
{
  long mtype; /* se usassimo mtype come identificatore di taxi pid? */
  int mtext[REQ_SIZE];
} Request;

/*
Messaggio di richiesta di spawn (per taxigen):
- pid = pid del taxi morto da rimuovere (se -1 viene ignorato)
- pos = posizione nella città
*/
typedef struct spawn
{
  long mtype;
  int mtext[2];
} Spawn;

#define SPAWN 2
#define RESPAWN 3

#define EXIT_TIMER EXIT_SUCCESS
#define EXIT_ERROR EXIT_FAILURE

typedef struct origin
{
  long mtype;
  int mtext[1];
} Origin;

#endif