#ifndef _TAXI_H
#define _TAXI_H

#include <sys/types.h>
#include "data_structures.h"

/* Imposta il signal handler di taxi */
void set_handler();

/* Inizializza dati IPC che servono globali */
void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems);

/* Inizializza altri dati globali */
void init_data(int master_pid, int pos);

/* Avvia il timer di SO_TIMEOUT */
void start_timer();

/* Receives new ride request */
extern void receive_ride_request(int requests_msq, Request *req);

#endif