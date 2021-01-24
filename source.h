#ifndef _SOURCE_H
#define _SOURCE_H

#include "data_structures.h"

/* coda dove ricevere origine */
int create_origin_msq();

/* Imposta il signal handler di source */
void set_handler();

/* Inizializza var. globali di source */
void init_data(int requests_msq, int city_id, int taxi_list_mem_id);

int generate_taxi_request(Request * req);

void send_taxi_request(Request * req);

/* Salva l'origine della sorgente */
void save_source_position(int origin_msq);

#endif