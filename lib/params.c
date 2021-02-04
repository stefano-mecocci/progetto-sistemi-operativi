#include "params.h"
#include <signal.h>
#include <stdio.h>

void check_params() {
  int max_holes = get_max_holes();

  if (SO_WIDTH < 2 && SO_HEIGHT < 2) {
    printf("Errore: città troppo piccola\n");
    raise(SIGTERM);
  }

  if (SO_CAP_MIN < 1 || SO_CAP_MAX < SO_CAP_MIN) {
    printf("Errore: SO_CAP_MAX deve essere maggiore SO_CAP_MIN ed entrambi maggiori di 1\n");
    raise(SIGTERM);
  }

  if (SO_SOURCES < 1) {
    printf("Errore: numero di sorgenti minore di 1\n");
    raise(SIGTERM);
  }

  if (SO_TOP_CELLS < 1) {
    printf("Errore: SO_TOP_CELLS minore di 1\n");
    raise(SIGTERM);
  }

  if (SO_TOP_CELLS > (SO_WIDTH * SO_HEIGHT - SO_HOLES)) {
    printf("Errore: SO_TOP_CELLS maggiore delle celle totali disponibili\n");
    raise(SIGTERM);
  }

  if (SO_SOURCES > (SO_WIDTH * SO_HEIGHT - SO_HOLES)) {
    printf("Errore: troppe sorgenti o città troppo piccola\n");
    raise(SIGTERM);
  }

  if (SO_HOLES > max_holes) {
    printf("Errore: troppe buche; max=%d\n", max_holes);
    raise(SIGTERM);
  }

  if (SO_CAP_MIN > SO_CAP_MAX) {
    printf("Errore: SO_CAP_MIN maggiore di SO_CAP_MAX\n");
    raise(SIGTERM);
  }

  if (SO_TIMENSEC_MIN < 0 || SO_TIMENSEC_MAX < SO_TIMENSEC_MIN) {
    printf("Errore: SO_TIMENSEC_MIN deve essere maggiore di SO_TIMENSEC_MAX ed entrambi interi positivi\n");
    raise(SIGTERM);
  }

  if (SO_TIMEOUT >= SO_DURATION) {
    printf("Errore: SO_TIMEOUT maggiore di SO_DURATION\n");
    raise(SIGTERM);
  }
}

void ensure_enough_taxi_capacity(int tot_capacity)
{
  if (SO_TAXI > tot_capacity)
  {
    printf("Errore: SO_TAXI maggiore della capacità totale estratta casualmente. \n");
    raise(SIGTERM);
  }
}

/* Get the max number of holes you can place in the city */
int get_max_holes()
{
  return GET_HOLE_RATIO(SO_WIDTH) * GET_HOLE_RATIO(SO_HEIGHT);
}