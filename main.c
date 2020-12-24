#include "params.h"
#include "master.h"
#include "data_structures.h"
#include <sys/types.h>

int main() {
  int city_id = create_city();
  int requests_id = create_requests_msq();
  int sync_sem = create_sync_sem();
  
  check_params();
  set_handler();

  init_city_cells(city_id);
  place_city_holes(city_id);
  init_stats();
  sem_set(sync_sem, 0, SO_TAXI);

  create_taxis();
  sem_wait_zero(sync_sem, 0);
  print_city(city_id);

  pause();


  return 0;
}