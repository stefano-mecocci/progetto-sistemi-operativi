#include "params.h"
#include "master.h"
#include "data_structures.h"

int main() {
  int city_id = create_city();
  int requests_id = create_requests_msq();
  
  check_params();
  set_handler();

  init_city_cells(city_id);
  place_city_holes(city_id);
  init_stats();

  print_city(city_id);

  pause();


  return 0;
}