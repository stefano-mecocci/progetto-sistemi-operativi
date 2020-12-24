#ifndef _MASTER_H
#define _MASTER_H

#include <sys/types.h>

/*
Controlla che i parametri siano validi
*/
void check_params();

/*
Crea la città in memoria condivisa
salva l'ID nella var globale g_city_id
ritorna l'ID
*/
int create_city();

/*
Inizializza tutte le celle come normali
*/
void init_city_cells(int city_id);

/*
Pulisce la memoria dagli oggetti IPC usati
*/
void clear_ipc_memory();

/*
Imposta il signal handler per gestire:
- SIGINT -> pulisce memoria IPC e termina
*/
void set_handler();

/* Stampa la città in ASCII sul terminale */
void print_city(int city_id);

/* Posiziona le buche nella città */
void place_city_holes(int city_id);

/* Inizializzazione statistiche */
void init_stats();

/* Crea coda di richieste per i taxi */
int create_requests_msq();

/* Crea i processi taxi */
void create_taxis();

/* 
Crea il semaforo per far sincronizzare:
- master e taxi
-master e richieste
*/
int create_sync_sem();

/* Imposta il sem in sem_arr a value */
void sem_set(int sem_arr, int sem, int value);

void sem_wait_zero(int sem_arr, int sem);

#endif