#ifndef _SOURCE_LIB_H
#define _SOURCE_LIB_H

#include "data_structures.h"

/* Set source signal handler */
void set_handler();

/* Initialize source global variables */
void init_data(int requests_msq, int city_id);

/* Generate a new taxi request message */
int generate_taxi_request(RequestMsg *req);

/* Sends a new taxi request message on the requests_msq */
void send_taxi_request(RequestMsg *req);

/* Save source position/origin in global var */
void set_source_position(int position);

#endif