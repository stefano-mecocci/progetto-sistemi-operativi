#include "data_structures.h"
#include "source.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  int sync_sems = read_id_from_file("sync_sems");
  int requests_msq = read_id_from_file("requests_msq");
  int city_id = read_id_from_file("city_id");
  int origin_msq = create_origin_msq();
  Request req;

  set_handler();
  init_data(requests_msq, city_id);
  sem_decrease(sync_sems, SEM_SYNC_SOURCES, -1, 0);
  save_source_position(origin_msq);

  while (TRUE) {
    sleep_for(3, 0);
    generate_taxi_request(&req);
    send_taxi_request(&req);
  }

  return 0;
}
