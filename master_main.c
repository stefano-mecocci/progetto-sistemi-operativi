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
  int city_sems_op = create_city_sems_op();
  int city_sems_cap = create_city_sems_cap();
  int requests_msq = create_requests_msq();
  int taxi_info_msq = create_taxi_info_msq();
  int taxi_spawn_msq = create_taxi_spawn_msq();

  write_id_to_file(city_id, "city_id");
  write_id_to_file(sync_sems, "sync_sems");
  write_id_to_file(city_sems_op, "city_sems_op");
  write_id_to_file(city_sems_cap, "city_sems_cap");
  write_id_to_file(requests_msq, "requests_msq");
  write_id_to_file(taxi_spawn_msq, "taxi_spawn_msq");
  write_id_to_file(taxi_info_msq, "taxi_info_msq");

  check_params();
  init_data();
  set_handler();

  print_pid();

  init_city_cells(city_id);
  place_city_holes(city_id);

  init_sync_sems(sync_sems);
  init_city_sems_op(city_sems_op);
  init_city_sems_cap(city_id, city_sems_cap);

  set_sources(city_id, city_sems_op);
  create_sources();
  err = sem_op(sync_sems, SEM_SYNC_SOURCES, 0, 0);
  DEBUG_RAISE_INT(err);
  send_sources_origins();


  create_taxigen();
  create_taxis(taxi_spawn_msq);
  err = sem_op(sync_sems, SEM_SYNC_TAXI, 0, 0);
  DEBUG_RAISE_INT(err);
  
  
  start_timer();
  start_change_detector();

  while (TRUE) {
    sleep_for(PRINT_INTERVAL, 0);
    print_city(city_id);
  }

  return 0;
}