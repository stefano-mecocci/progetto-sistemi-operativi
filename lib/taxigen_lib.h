#ifndef _TAXIGEN_LIB_H
#define _TAXIGEN_LIB_H

#include <sys/types.h>
#include "data_structures.h"

/* Set taxigen signal handler */
void set_handler();

/* Initialize taxigen global data/variables */
void init_data();

/* Receive(wait for) a taxi spawn request */
void receive_spawn_request(int taxi_spawn_msq, SpawnMsg *req);

/* Create taxi process with args: master pid and (bool) is respawned */
pid_t create_taxi(int isNew);

/* Add pid to global array g_taxi_pids */
void add_taxi_pid(pid_t pid);

/* Replace pid in global array g_taxi_pids */
void replace_taxi_pid(pid_t old_pid, pid_t new_pid);

#endif