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

/* Imposta le buche della città */
void set_city_holes(int city_id);

#endif