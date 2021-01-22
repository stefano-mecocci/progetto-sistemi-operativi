#include "utils.h"
#include "data_structures.h"
#include "params.h"
#include <stdlib.h>

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