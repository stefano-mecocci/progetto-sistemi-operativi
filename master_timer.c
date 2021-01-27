#include "params.h"
#include "utils.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>


int main(int argc, char const *argv[]) {
  sleep_for(SO_DURATION, 0);
  kill(getppid(), SIGUSR2);

  return 0;
}
