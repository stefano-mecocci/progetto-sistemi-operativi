#include "lib/params.h"
#include "lib/utils.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
  sleep_for(SO_DURATION, 1000); /* add 1 usec for let the master finish to print the map */
  kill(getppid(), SIGUSR2);

  return 0;
}
