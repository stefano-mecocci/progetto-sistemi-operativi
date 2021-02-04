#ifndef _MASTER_LIB_H
#define _MASTER_LIB_H

#include <sys/types.h>

/*
Create the city in IPC shared memory
(see data_structures.h)
*/
extern int create_city();

/*
Create the IPC semaphore array for syncronization
of length 2

- 0 : syncronize created taxis
- 1 : syncronize created sources
*/
extern int create_sync_sems();

/*
Create the IPC semaphore array for cells capacity, 
with length SO_WIDTH * SO_HEIGHT
*/
extern int create_city_sems_cap();

/*
Create requests IPC message queue
*/
extern int create_requests_msq();

/*
Create IPC message queue for taxi updates
*/
extern int create_taxi_info_msq();

/*
Create the IPC message queue to spawn taxi
*/
extern int create_taxi_spawn_msq();

/*
Check if configuration parameters are correct
*/
extern void check_params();

/*
Initialize global data of master:
- processes pids (sources, taxigen, change_detector)
- other (g_source_positions)
*/
extern void init_data();

/*
Sets the master signal handler for:
- SIGINT (debug)
- SIGTERM (debug)
- SIGUSR1 (ask sources to generate a taxi request)
- SIGUSR2 (simulation timer)
*/
extern void set_handler();

/*
Initialize the city cells (see data_structures.h)
*/
extern void init_city_cells(int city_id);

/*
Initialize syncronization semaphores
- 0 : SO_TAXI
- 1 : SO_SOURCES
*/
extern void init_sync_sems(int sync_sems);

/*
Initialize capacity semaphores to capacity from cell
*/
extern void init_city_sems_cap(int city_id, int city_sems_cap);

/*
Place SO_HOLES holes in the city
*/
extern void place_city_holes(int city_id);

/*
Create taxigen process
*/
extern void create_taxigen();

/*
Sends SO_TAXI messages to create taxi processes
*/
extern void create_taxis(int taxi_spawn_msq);

/*
Create SO_SOURCES source processes with random position for each
*/
extern void create_sources();

/*
Start master timer of SO_DURATION seconds
*/
extern void start_timer();

/*
Create change detector process
*/
extern void start_change_detector();

#endif