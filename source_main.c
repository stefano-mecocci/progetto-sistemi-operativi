#include "data_structures.h"
#include "source.h"
#include "params.h"
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
  int origin = atoi(argv[1]);
  RequestMsg req;

	srand(time(NULL)^getpid());

  set_handler();
  init_data(requests_msq, city_id);
  err = sem_op(sync_sems, SEM_SYNC_SOURCES, -1, 0);
  DEBUG_RAISE_INT(err);
  set_source_position(origin);

  while (TRUE) {
    generate_taxi_request(&req);
    send_taxi_request(&req);
    sleep_for(RIDE_REQUEST_INTERVAL, 0);
  }

  return 0;
}
