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

#endif