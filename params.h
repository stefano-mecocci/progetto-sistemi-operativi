#ifndef _PARAMS_H
#define _PARAMS_H

#include <stdlib.h>

/* Larghezza griglia città */
#define SO_WIDTH 30

/* Altezza griglia città */
#define SO_HEIGHT 30

/* sec */
#define PRINT_INTERVAL 1

/* sec */
#define RIDE_REQUEST_INTERVAL 3

/* Numero di buche nella città */
#define SO_HOLES (atoi(getenv("SO_HOLES")))

/* Numero di celle maggiormente attraversate */
#define SO_TOP_CELLS (atoi(getenv("SO_TOP_CELLS")))

/* Numero di sorgenti */
#define SO_SOURCES (atoi(getenv("SO_SOURCES")))

/* Capacità minima di una cella */
#define SO_CAP_MIN (atoi(getenv("SO_CAP_MIN")))

/* Capacità massima di una cella */
#define SO_CAP_MAX (atoi(getenv("SO_CAP_MAX")))

/* Numero di taxi in città */
#define SO_TAXI (atoi(getenv("SO_TAXI")))

/* Tempo minimo per attraversamento di una cella */
#define SO_TIMENSEC_MIN (atoi(getenv("SO_TIMENSEC_MIN")))

/* Tempo massimo per attraversamento di una cella */
#define SO_TIMENSEC_MAX (atoi(getenv("SO_TIMENSEC_MAX")))

/* Timeout del taxi */
#define SO_TIMEOUT (atoi(getenv("SO_TIMEOUT")))

/* Durata massima della simulazione */
#define SO_DURATION (atoi(getenv("SO_DURATION")))

#endif