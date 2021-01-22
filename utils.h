#include "data_structures.h"

/* Conversione indice -> punto */
extern Point index2point(int index);


/* Conversione punto -> indice */
extern int point2index(Point p);

/* Returns taxicab distance between map indexes */
extern int indexes_delta(int, int);


/* Returns taxicab distance between map points */
extern int points_delta(Point, Point);