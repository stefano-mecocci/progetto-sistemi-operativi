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
  int steps = 0, started = 0;
  
  init_data_ipc(taxi_spawn_msq, taxi_info_msq, sync_sems, city_id, city_sems_cap, requests_msq);
  init_data(atoi(argv[2]), atoi(argv[3]));
  set_handler();

  if (atoi(argv[1]) == FALSE) {
    err = sem_op(sync_sems, SEM_SYNC_TAXI, -1, 0);
    DEBUG_RAISE_INT(err);
  }

  init_astar();
  /* start_timer(); */

  while(TRUE){
    set_aborted_request(FALSE);
    receive_ride_request(&req);
    set_aborted_request(TRUE);
    /* Stop taxi timer */
    /* reset_taxi_timer(); */
    if(req.mtext.origin != get_position()){
      /* gather path to source */
      path = get_path(get_position(), req.mtext.origin, &steps);
      travel(path, steps);
      if(get_position() != req.mtext.origin){
        errno = 0;
        printf("Taxi %d did not reach the correct source for pickup.\n", getpid());
        raise(SIGUSR1);
      }
    }
    printf("START RIDE\n");
    status.available = FALSE;
    status.pid = getpid();
    status.position = get_position();
    send_taxi_update(taxi_info_msq, PICKUP, status);
    /* gather path to destination */
    path = get_path(get_position(), req.mtext.destination, &steps);
    travel(path, steps);
    printf("END RIDE\n");
    status.available = TRUE;
    status.pid = getpid();
    status.position = get_position();
    send_taxi_update(taxi_info_msq, SERVED, status);
  }

  return 0;
}
