#include <stdio.h>
#include "taxi.h"

int main() {
  int sync_sem = get_sync_sem();

  set_handler();

  printf("Hello world\n");

  sem_remove(sync_sem, 0, -1);

  return 0;
}