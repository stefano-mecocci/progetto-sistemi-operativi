#ifndef _TAXIGEN_H
#define _TAXIGEN_H

#include <sys/types.h>
#include "data_structures.h"

/* Imposta il signal handler di taxigen */
void set_handler();

/* Inizializza i dati globali di taxigen */
void init_data();

/* Riceve una richiesta di spawn di taxi */
void receive_spawn_request(int taxi_spawn_msq, Spawn *req);

/*
Siccome il taxi è morto, incrementa la capacità attuale della
cella
*/
void remove_old_taxi(int city_sems_cap, int pos);

/*
Imposta un taxi sulla città, in una posizione casuale, riducendo
la capacità attuale della cella
*/
int set_taxi(int city_id, int city_sems_cap);

/* Crea un processo taxi passandogli la posizione e il master pid */
pid_t create_taxi(int pos, int isNew);

/* Aggiunge un pid all'array globale g_taxi_pids */
void add_taxi_pid(pid_t pid);

/* Rimpiazza un pid nell'array global g_taxi_pids */
void replace_taxi_pid(pid_t old_pid, pid_t new_pid);

#endif