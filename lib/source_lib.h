#ifndef _SOURCE_H
#define _SOURCE_H

#include "data_structures.h"

/* Imposta il signal handler di source */
void set_handler();

/* Inizializza var. globali di source */
void init_data(int requests_msq, int city_id);

int generate_taxi_request(RequestMsg *req);

void send_taxi_request(RequestMsg *req);

/* Salva l'origine della sorgente */
void set_source_position(int position);

#endif