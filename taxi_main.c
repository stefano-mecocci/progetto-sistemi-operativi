#define _GNU_SOURCE

#include "data_structures.h"
#include "params.h"
#include "taxi.h"
#include "utils.h"
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
  int taxi_spawn_msq = read_id_from_file("taxi_spawn_msq");
  int taxi_info_msq = read_id_from_file("taxi_info_msq");
  int requests_msq = read_id_from_file("requests_msq");
  int sync_sems = read_id_from_file("sync_sems");
  int city_sems_op = read_id_from_file("city_sems_op");
  int city_sems_cap = read_id_from_file("city_sems_cap");
  int city_id = read_id_from_file("city_id");
  pid_t timer_pid;
  Request req;
  
  init_data_ipc(taxi_spawn_msq, taxi_info_msq, sync_sems);
  init_data(atoi(argv[2]), atoi(argv[3]));
  set_handler();

  if (atoi(argv[1]) == FALSE) {
    sem_decrease(sync_sems, SEM_SYNC_TAXI, -1, IPC_NOWAIT);
  }

  timer_pid = start_timer();

  /* while (1); */
  while(TRUE){
    receive_ride_request(requests_msq, &req);
  }

  return 0;
}