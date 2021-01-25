#ifndef _TAXI_H
#define _TAXI_H

#include <sys/types.h>
#include "data_structures.h"
#include "astar/astar.h"

/* Imposta il signal handler di taxi */
extern void set_handler();

/* Inizializza dati IPC che servono globali */
extern void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems, int city_id);

/* Inizializza altri dati globali */
extern void init_data(int master_pid, int pos);

/* Avvia il timer di SO_TIMEOUT */
extern void start_timer();

/* Receives new ride request */
extern void receive_ride_request(int requests_msq, RequestMsg *req);

extern int get_position();

extern void init_astar();

extern direction_t *get_path(int position, int destination);

extern void travel(direction_t *directions);

#endif