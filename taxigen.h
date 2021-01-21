#ifndef _TAXIGEN_H
#define _TAXIGEN_H

#include <sys/types.h>
#include "data_structures.h"

/* Legge l'id di una risorsa IPC da un file */
int read_id_from_file(char * filename);

/* Imposta il signal handler di taxigen */
void set_handler();

/* Inizializza i dati globali di taxigen */
void init_data();

/* Riceve una richiesta di spawn di taxi */
void receive_spawn_request(int taxi_spawn_msq, Spawn *req);

/* Sistema la capacità della cella dato che il taxi è morto */
void remove_old_taxi(int city_id, int city_sems_op, int city_sems_cap, int pos);

/* Autoesplicativo */
int sem_decrease(int sem_arr, int sem, int value, short flag);

/* Autoesplicativo */
int sem_increase(int sem_arr, int sem, int value, short flag);

/* Imposta un taxi su pos, modificando la capacità */
int set_taxi(int city_id, int city_sems_op, int city_sems_cap);

/* Crea un processo taxi passandogli la posizione e il master pid */
pid_t create_taxi(int pos, int isNew);

/* Aggiunge un pid all'array globale g_taxi_pids */
void add_taxi_pid(pid_t pid);

/* RImpiazza un pid nell'array global g_taxi_pids */
void replace_taxi_pid(pid_t old_pid, pid_t new_pid);

#endif