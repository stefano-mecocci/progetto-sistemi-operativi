#define _GNU_SOURCE

#include "lib/data_structures.h"
#include "lib/params.h"
#include "lib/taxi_lib.h"
#include "lib/utils.h"
#include "lib/astar/astar.h"
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

int main(int argc, char const *argv[])
{
  int err;
  int taxi_spawn_msq = read_id_from_file(IPC_TAXI_SPAWN_MSQ_FILE);
  int taxi_info_msq = read_id_from_file(IPC_TAXI_INFO_MSQ_FILE);
  int requests_msq = read_id_from_file(IPC_REQUESTS_MSQ_FILE);
  int sync_sems = read_id_from_file(IPC_SYNC_SEMS_FILE);
  int city_sems_cap = read_id_from_file(IPC_CITY_SEMS_CAP_FILE);
  int city_id = read_id_from_file(IPC_CITY_ID_FILE);
  int is_respawned = atoi(argv[1]);
  RequestMsg req;
  TaxiStatus status;
  direction_t *path;
  long last_travel_duration;
  int steps = 0;

  init_data_ipc(taxi_spawn_msq, taxi_info_msq, sync_sems, city_id, city_sems_cap, requests_msq);
  init_data(atoi(argv[2]), atoi(argv[3]));
  set_handler();
  copy_city();

  if (is_respawned == FALSE)
  {
    err = sem_op(sync_sems, SEM_SYNC_TAXI, -1, 0);
    DEBUG_RAISE_INT(err);
  }

  status.longest_travel_time = 0;
  init_astar();
  start_timer();

  while (TRUE)
  {
    set_aborted_request(FALSE);
    receive_ride_request(&req);
    set_aborted_request(TRUE);

    reset_stopwatch();

    if (req.mtext.origin != get_position())
    {
      /* gather path to source */
      path = get_path(get_position(), req.mtext.origin, &steps);
      travel(path, steps);
      if (get_position() != req.mtext.origin)
      {
        errno = 0;
        printf("Taxi %d did not reach the correct source for pickup.\n", getpid());
        raise(SIGALRM);
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
    if (get_position() != req.mtext.origin)
    {
      errno = 0;
      printf("Taxi %d did not reach the correct destination.\n", getpid());
      raise(SIGALRM);
    }
    printf("END RIDE\n");
    last_travel_duration = record_stopwatch();
    status.available = TRUE;
    status.pid = getpid();
    status.position = get_position();

    if (last_travel_duration > status.longest_travel_time)
    {
      status.longest_travel_time = last_travel_duration;
    }

    send_taxi_update(taxi_info_msq, SERVED, status);
  }

  return 0;
}
