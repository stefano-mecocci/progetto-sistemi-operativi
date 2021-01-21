#ifndef _SOURCE_H
#define _SOURCE_H

#include "data_structures.h"

/* coda dove ricevere origine */
int create_origin_msq();

/* Legge id da file */
int read_id_from_file(char *filename);

/* Rimuovi value dal semaforo sem in sem_arr */
int sem_decrease(int sem_arr, int sem, int value, short flag);

/* Imposta il signal handler di source */
void set_handler();

/* Inizializza var. globali di source */
void init_data(int requests_msq, int city_id);

void generate_taxi_request(Request * req);

void send_taxi_request(Request * req);

/* Salva l'origine della sorgente */
void save_source_position(int origin_msq);

#endif