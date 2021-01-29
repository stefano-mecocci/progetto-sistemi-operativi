#ifndef _TAXI_H
#define _TAXI_H

#include <sys/types.h>
#include "data_structures.h"
#include "astar/astar.h"

/* Imposta il signal handler di taxi */
extern void set_handler();

/* Inizializza dati IPC che servono globali */
extern void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems, int city_id, int city_sems_cap, int requests_msq);

/* Inizializza altri dati globali */
extern void init_data(int master_pid, int pos);

extern void copy_city();

/* Avvia il timer di SO_TIMEOUT */
extern void create_timer();

extern void start_timer();

extern void reset_taxi_timer();

/* Receives new ride request */
extern void receive_ride_request(RequestMsg *req);

extern int get_position();

extern void set_position(int addr);

extern void set_aborted_request(enum Bool);

extern void init_astar();

extern direction_t *get_path(int position, int destination, int *steps);

extern void travel(direction_t *directions, int steps);

extern void record();

#endif