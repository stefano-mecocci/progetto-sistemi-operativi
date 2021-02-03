#include "lib/params.h"
#include "lib/utils.h"
#include <signal.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  /* add 1 usec to let master finish to print the map */
  sleep_for(SO_DURATION, 1000);
  kill(getppid(), SIGUSR2);

  return 0;
}
