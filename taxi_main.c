#include <stdio.h>
#include "data_structures.h"
#include "taxi.h"

int main() {
  int sync_sem = get_sync_sem();
  int requests_msq = get_requests_msq();

  set_handler();
  sem_decrease(sync_sem, 0, -1);

  while (TRUE) {
    take_request(requests_msq);
  }

  return 0;
}