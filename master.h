#ifndef _MASTER_H
#define _MASTER_H

#include <sys/types.h>

/* Città array in shm */
extern int create_city();

/* Array semafori per sincronizzazioni */
extern int create_sync_sems();

/* Array semafori per operare su celle */
extern int create_city_sems_op();

/* Array semafori per controllare la capacità */
extern int create_city_sems_cap();

/* Coda per richieste taxi */
extern int create_requests_msq();

/* Coda per info taxi per statistiche */
extern int create_taxi_info_msq();

/* Coda per (re)spawnare taxi */
extern int create_taxi_spawn_msq();

/* Controlla i parametri */
extern void check_params();

/*
Inizializza i dati globali:
- entità (source_pids e taxigen_pid)
- statistiche (g_travels, g_top_cells e taxi vari)
*/
extern void init_data();

/* Imposta il signal handler di master */
extern void set_handler();

/* Inizializza le celle della città */
extern void init_city_cells(int city_id);

/* Inizializza semafori di sincronizzazione */
extern void init_sync_sems(int sync_sems);

/* Inizializza semafori "modificabile" a 1 = modificabile */
extern void init_city_sems_op(int city_sems_op);

/* Inizializza i semafori capacità alla cap. collegata */
extern void init_city_sems_cap(int city_id, int city_sems_cap);

/* Piazza SO_HOLES buche nella città */
extern void place_city_holes(int city_id);

/* Crea il processo taxigen */
extern void create_taxigen();

/* Manda SO_TAXI messaggi per far creare i relativi processi */
extern void create_taxis(int taxi_spawn_msq);

/* Avvio il timer di master di SO_DURATION secondi */
extern void start_timer();

/* Crea i processi sorgente */
extern void create_sources();

#endif