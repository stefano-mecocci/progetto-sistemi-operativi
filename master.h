#ifndef _MASTER_H
#define _MASTER_H

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
pid_t *create_taxis();

#endif