#include "params.h"
#include "utils.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>


int main(int argc, char const *argv[]) {
  sleep_for(0, 100000000); /* fix temporaneo */

  sleep_for(SO_DURATION, 0);
  kill(getppid(), SIGUSR2);

  return 0;
}
