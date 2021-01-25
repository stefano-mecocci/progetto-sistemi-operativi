#include "data_structures.h"
#include "source.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char const *argv[]) {
  int err;
  int sync_sems = read_id_from_file("sync_sems");
  int requests_msq = read_id_from_file("requests_msq");
  int city_id = read_id_from_file("city_id");
  int taxi_list_mem_id = read_id_from_file("taxi_list_id");
  int origin_msq = create_origin_msq();
  RequestMsg req;

  set_handler();
  init_data(requests_msq, city_id, taxi_list_mem_id);
  err = sem_op(sync_sems, SEM_SYNC_SOURCES, -1, 0);
  DEBUG_RAISE_INT(err);
  save_source_position(origin_msq);

  while (TRUE) {
    sleep_for(3, 0);
    generate_taxi_request(&req);
    send_taxi_request(&req);
  }

  return 0;
}
