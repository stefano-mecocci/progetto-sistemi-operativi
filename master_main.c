#define _GNU_SOURCE

#include "data_structures.h"
#include "master.h"
#include "params.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>

#define DEBUG                                                                  \
  printf("ERRNO: %d at line %d in file %s\n", errno, __LINE__, __FILE__);

void print_pid() { printf("\n[MASTER] pid %d\n\n", getpid()); }

int sleep_for(int secs, int nanosecs) {
  struct timespec t;
  int err;

  t.tv_sec = secs;
  t.tv_nsec = nanosecs;

  err = nanosleep(&t, NULL);

  return err;
}

int main() {
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
  write_id_to_file(taxi_info_msq, "taxi_info_msq");
  write_id_to_file(requests_msq, "requests_msq");
  write_id_to_file(taxi_spawn_msq, "taxi_spawn_msq");

  check_params();
  init_data();
  set_handler();

  print_pid();

  init_city_cells(city_id);
  place_city_holes(city_id);

  init_sync_sems(sync_sems);
  init_city_sems_op(city_sems_op);
  init_city_sems_cap(city_id, city_sems_cap);

  create_taxigen();
  create_taxis(taxi_spawn_msq);
  sem_wait_zero(sync_sems, SEM_SYNC_TAXI, 0);

  set_sources(city_id, city_sems_op);
  create_sources();
  sem_wait_zero(sync_sems, SEM_SYNC_SOURCES, 0);
  send_sources_origins();
  
  start_timer();

  while (TRUE) {
    sleep_for(1, 0);
    print_city(city_id);
  }

  return 0;
}