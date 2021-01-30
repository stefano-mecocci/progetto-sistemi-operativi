#include "params.h"
#include <signal.h>
#include <stdio.h>

void check_params()
{
  if (SO_SOURCES > (SO_WIDTH * SO_HEIGHT - SO_HOLES))
  {
    printf("Errore: troppe sorgenti, città troppo piccola o troppe buche\n");
    raise(SIGTERM);
  }

  if (SO_CAP_MIN > SO_CAP_MAX)
  {
    printf("Errore: SO_CAP_MIN maggiore di SO_CAP_MAX\n");
    raise(SIGTERM);
  }

  if (SO_TIMENSEC_MIN > SO_TIMENSEC_MAX)
  {
    printf("Errore: SO_TIMENSEC_MIN maggiore di SO_TIMENSEC_MAX\n");
    raise(SIGTERM);
  }

  if (SO_TIMEOUT >= SO_DURATION)
  {
    printf("Errore: SO_TIMEOUT maggiore di SO_DURATION\n");
    raise(SIGTERM);
  }
}