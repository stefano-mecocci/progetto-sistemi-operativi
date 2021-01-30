#include "data_structures.h"
#include "master.h"
#include "params.h"
#include "utils.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>

void print_pid() { printf("\n[MASTER] pid %d\n\n", getpid()); }

int main() {
  int err;
  int city_id = create_city();
  int sync_sems = create_sync_sems();
  int city_sems_cap = create_city_sems_cap();
  int requests_msq = create_requests_msq();
  int taxi_info_msq = create_taxi_info_msq();
  int taxi_spawn_msq = create_taxi_spawn_msq();

  write_id_to_file(city_id, IPC_CITY_ID_FILE);
  write_id_to_file(sync_sems, IPC_SYNC_SEMS_FILE);
  write_id_to_file(city_sems_cap, IPC_CITY_SEMS_CAP_FILE);
  write_id_to_file(requests_msq, IPC_REQUESTS_MSQ_FILE);
  write_id_to_file(taxi_spawn_msq, IPC_TAXI_SPAWN_MSQ_FILE);
  write_id_to_file(taxi_info_msq, IPC_TAXI_INFO_MSQ_FILE);

  check_params();
  init_data();
  set_handler();

  print_pid();

  init_city_cells(city_id);
  place_city_holes(city_id);

  init_sync_sems(sync_sems);
  init_city_sems_cap(city_id, city_sems_cap);

  create_sources();
  err = sem_op(sync_sems, SEM_SYNC_SOURCES, 0, 0);
  DEBUG_RAISE_INT(err);

  create_taxigen();
  create_taxis(taxi_spawn_msq);
  err = sem_op(sync_sems, SEM_SYNC_TAXI, 0, 0);
  DEBUG_RAISE_INT(err);
  
  start_timer();
  start_change_detector();

  while (TRUE) {
    sleep_for(PRINT_INTERVAL, 0);
    print_city(stdout, city_id, city_sems_cap, ACT_CAPACITY, NULL);
  }

  return 0;
}