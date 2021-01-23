#define _GNU_SOURCE

#include "data_structures.h"
#include "params.h"
#include "taxigen.h"

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

int main() {
  int taxi_spawn_msq = read_id_from_file("taxi_spawn_msq");
  int sync_sems = read_id_from_file("sync_sems");
  int city_id = read_id_from_file("city_id");
  int city_sems_cap = read_id_from_file("city_sems_cap");
  int city_sems_op = read_id_from_file("city_sems_op");
  int pos;
  Spawn req;
  pid_t taxi_pid;

  set_handler();
  init_data();

  while (TRUE) {
    receive_spawn_request(taxi_spawn_msq, &req);
    sem_increase(sync_sems, SEM_ALIVES_TAXI, 1, 0);

    if (req.mtype == RESPAWN) {
      remove_old_taxi(city_sems_cap, req.mtext[1]);
    }

    pos = set_taxi(city_id, city_sems_cap);

    if (req.mtype == RESPAWN) {
      taxi_pid = create_taxi(pos, TRUE);
    } else {
      taxi_pid = create_taxi(pos, FALSE);
    }

    if (req.mtype == RESPAWN) {
      replace_taxi_pid(req.mtext[0], taxi_pid);
    } else {
      add_taxi_pid(taxi_pid);
    }
  }

  return 0;
}
