#ifndef _PARAMS_H
#define _PARAMS_H

#include <stdlib.h>

/* Larghezza griglia città */
#define SO_WIDTH 20

/* Altezza griglia città */
#define SO_HEIGHT 10

#define GET_HOLE_RATIO(axis) ((int)(axis / 2) + (int)(axis % 2))

/* seconds between calls of print_city */
#define PRINT_INTERVAL 1

/* seconds between generation of taxi request */
#define RIDE_REQUEST_INTERVAL 3

/* number of holes in the city */
#define SO_HOLES (atoi(getenv("SO_HOLES")))

/* number of most crossed cells */
#define SO_TOP_CELLS (atoi(getenv("SO_TOP_CELLS")))

/* number of sources */
#define SO_SOURCES (atoi(getenv("SO_SOURCES")))

/* Minimum cell capacity */
#define SO_CAP_MIN (atoi(getenv("SO_CAP_MIN")))

/* Maximum cell capacity */
#define SO_CAP_MAX (atoi(getenv("SO_CAP_MAX")))

/* number of taxis */
#define SO_TAXI (atoi(getenv("SO_TAXI")))

/* Minimum time to cross a cell */
#define SO_TIMENSEC_MIN (atoi(getenv("SO_TIMENSEC_MIN")))

/* Maximum time to cross a cell */
#define SO_TIMENSEC_MAX (atoi(getenv("SO_TIMENSEC_MAX")))

/* seconds of timeout for a taxi */
#define SO_TIMEOUT (atoi(getenv("SO_TIMEOUT")))

/* Max simulation duration (a.k.a. timer) */
#define SO_DURATION (atoi(getenv("SO_DURATION")))

/* Check if the config params are valid */
extern void check_params();

/* Get maximum number of placeable holes */
extern int get_max_holes();

#endif