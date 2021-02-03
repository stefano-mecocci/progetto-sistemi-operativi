#ifndef _TAXI_H
#define _TAXI_H

#include <sys/types.h>
#include "data_structures.h"
#include "astar/pathfinder.h"

/* Set taxi signal handler */
extern void set_handler();

/* Initialize global variables of IPC ids */
extern void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems, int city_id, int city_sems_cap, int requests_msq);

/* Initialize global data of taxi */
extern void init_data(int master_pid, int pos);

/* Find the first random free spot on map for first positioning */
extern int set_taxi(int city_id, int city_sems_cap);

extern void copy_city();

/* Starts the taxi timer of SO_TIMEOUT seconds */
extern void start_timer();

/* Receives new ride request */
extern void receive_ride_request(RequestMsg *req);

/* Get actual taxi position (global var) */
extern int get_position();

/* Set actual taxi position (global var) */
extern void set_position(int addr);

extern void set_aborted_request(enum Bool);

extern void init_astar();

extern AStar_Node *get_path(int position, int destination);

extern void travel(AStar_Node *navigator);

#endif