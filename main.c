#include "params.h"
#include "master.h"
#include "data_structures.h"
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>

int main() {
  int city_id = create_city();
  int requests_msq = create_requests_msq();
  int sync_sem = create_sync_sem();
  int master_msq = create_master_msq();
  int city_sems = create_city_sems();
  
  check_params();
  set_handler();

  init_city_cells(city_id);
  /* init_city_sems(city_sems); */
  place_city_holes(city_id);
  init_stats();
  sem_set(sync_sem, 0, SO_TAXI);

  create_taxis();
  sem_wait_zero(sync_sem, 0);
  print_city(city_id);

  pause();


  return 0;
}