#define _GNU_SOURCE

#include "data_structures.h"
#include "params.h"
#include "taxi.h"
#include "utils.h"
#include "astar/astar.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  int err;
  int taxi_spawn_msq = read_id_from_file("taxi_spawn_msq");
  int taxi_info_msq = read_id_from_file("taxi_info_msq");
  int requests_msq = read_id_from_file("requests_msq");
  int sync_sems = read_id_from_file("sync_sems");
  int city_sems_op = read_id_from_file("city_sems_op");
  int city_sems_cap = read_id_from_file("city_sems_cap");
  int city_id = read_id_from_file("city_id");
  RequestMsg req;
  TaxiStatus status;
  direction_t *path;
  
  init_data_ipc(taxi_spawn_msq, taxi_info_msq, sync_sems, city_id);
  init_data(atoi(argv[2]), atoi(argv[3]));
  set_handler();

  if (atoi(argv[1]) == FALSE) {
    err = sem_op(sync_sems, SEM_SYNC_TAXI, -1, 0);
    DEBUG_RAISE_INT(err);
  }

  /* start_timer(); */
  init_astar();

  while(TRUE){
    receive_ride_request(requests_msq, &req);
    printf("Received new ride request: source=%d; destination=%d", req.mtext.origin, req.mtext.destination);
    /* Stop taxi timer */
    if(req.mtext.origin != get_position()){
      printf("First moving to source for pickup\n");
      /* gather path to source */
      path = get_path(get_position(), req.mtext.origin);
      travel(path);
    }
    status.available = FALSE;
    status.pid = getpid();
    status.position = get_position();
    send_taxi_update(taxi_info_msq, PICKUP, status);
    /* gather path to destination */
    path = get_path(get_position(), req.mtext.destination);
    travel(path);    
    status.available = TRUE;
    status.pid = getpid();
    status.position = get_position();
    send_taxi_update(taxi_info_msq, SERVED, status);
  }

  return 0;
}