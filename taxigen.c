#define _GNU_SOURCE

#include "lib/data_structures.h"
#include "lib/params.h"
#include "lib/taxigen_lib.h"
#include "lib/utils.h"

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

int main()
{
  int taxi_spawn_msq = read_id_from_file(IPC_TAXI_SPAWN_MSQ_FILE);
  int taxi_info_msq_id = read_id_from_file(IPC_TAXI_INFO_MSQ_FILE);
  int sync_sems = read_id_from_file(IPC_SYNC_SEMS_FILE);
  int city_id = read_id_from_file(IPC_CITY_ID_FILE);
  int city_sems_cap = read_id_from_file(IPC_CITY_SEMS_CAP_FILE);
  int err;
  SpawnMsg req;
  TaxiStatus status;
  pid_t taxi_pid;

  set_handler();
  init_data();

  while (TRUE)
  {
    receive_spawn_request(taxi_spawn_msq, &req);

    if (req.mtype == RESPAWN)
    {
      taxi_pid = create_taxi(TRUE);
      replace_taxi_pid(req.mtext, taxi_pid);
    }
    else
    {
      taxi_pid = create_taxi(FALSE);
      add_taxi_pid(taxi_pid);
    }
  }

  return 0;
}
