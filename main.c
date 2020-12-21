#include <stdio.h>
#include "params.h"
#include "master.h"
#include "data_structures.h"

int main() {
  int city_id = create_city();
  
  check_params();
  set_handler();

  init_city_cells(city_id);

  pause();

  return 0;
}