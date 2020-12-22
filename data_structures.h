#ifndef _DATA_STRUCTURES_H
#define _DATA_STRUCTURES_H

/* Tipo di cella */
enum cell_type {
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
typedef struct cell {
  int type;
  int capacity;
  int cross_time;
} Cell;

/* Città = array di lunghezza SO_WIDTH * SO_HEIGHT */
typedef Cell * City;

/*
index2point {x: i % SO_WIDTH + 1, y: i / SO_WIDTH + 1}
point2index p.y * SO_WIDTH - 1
*/

/* 
Utile per calcolare sulla città
*/
typedef struct point {
  int x;
  int y;
} Point;

/* Struttura abbinata ad un processo taxi */
typedef struct taxi {
  int crossed_cells;
  int travel_time;
  int requests;
} Taxi;

#endif