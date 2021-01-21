#ifndef _MASTER_H
#define _MASTER_H

#include <sys/types.h>

/* Città array in shm */
int create_city();

/* Array semafori per sincronizzazioni */
int create_sync_sems();

/* Array semafori per operare su celle */
int create_city_sems_op();

/* Array semafori per controllare la capacità */
int create_city_sems_cap();

/* Coda per richieste taxi */
int create_requests_msq();

/* Coda per info taxi per statistiche */
int create_taxi_info_msq();

/* Coda per (re)spawnare taxi */
int create_taxi_spawn_msq();

/* Scrive l'id nel file filename */
void write_id_to_file(int id, char * filename);

/* Controlla i parametri */
void check_params();

/*
Inizializza i dati globali:
- entità (source_pids e taxigen_pid)
- statistiche (g_travels, g_top_cells e taxi vari)
*/
void init_data();

/* Imposta il signal handler di master */
void set_handler();

/* Inizializza le celle della città */
void init_city_cells(int city_id);

/* Inizializza semafori di sincronizzazione */
void init_sync_sems(int sync_sems);

/* Inizializza semafori "modificabile" a 1 = modificabile */
void init_city_sems_op(int city_sems_op);

/* Inizializza i semafori capacità alla cap. collegata */
void init_city_sems_cap(int city_id, int city_sems_cap);

/* Piazza SO_HOLES buche nella città */
void place_city_holes(int city_id);

/* Crea il processo taxigen */
void create_taxigen();

/* Manda SO_TAXI messaggi per far creare i relativi processi */
void create_taxis(int taxi_spawn_msq);

/* Avvio il timer di master di SO_DURATION secondi */
void start_timer();

/* Aspetta che sem sia 0 */
int sem_wait_zero(int sem_arr, int sem, short flag);

/* AUTOESPLICATIVO */
int sem_increase(int sem_arr, int sem, int value, short flag);

/* AUTOESPLICATIVO */
int sem_decrease(int sem_arr, int sem, int value, short flag);

/* Crea i processi sorgente */
void create_sources();

/* Stampa la città in ASCII */
void print_city(int city_id);

/* Imposta le celle sorgenti nella città */
void set_sources(int city_id, int city_sems_op);

/* Invia le posizioni di origine alle source */
void send_sources_origins();

#endif