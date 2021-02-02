#ifndef _TAXIGEN_H
#define _TAXIGEN_H

#include <sys/types.h>
#include "data_structures.h"

/* Imposta il signal handler di taxigen */
void set_handler();

/* Inizializza i dati globali di taxigen */
void init_data();

/* Riceve una richiesta di spawn di taxi */
void receive_spawn_request(int taxi_spawn_msq, SpawnMsg *req);

/* Crea un processo taxi passandogli la posizione e il master pid */
pid_t create_taxi(int isNew);

/* Aggiunge un pid all'array globale g_taxi_pids */
void add_taxi_pid(pid_t pid);

/* Rimpiazza un pid nell'array global g_taxi_pids */
void replace_taxi_pid(pid_t old_pid, pid_t new_pid);

#endif