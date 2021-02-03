#include "lib/data_structures.h"
#include "lib/params.h"
#include "lib/taxigen_lib.h"
#include "lib/utils.h"

#include <sys/types.h>

int main()
{
  int taxi_spawn_msq = read_id_from_file(IPC_TAXI_SPAWN_MSQ_FILE);
  SpawnMsg req;
  pid_t taxi_pid;

  set_handler();
  init_data();

  while (TRUE)
  {
    receive_spawn_request(taxi_spawn_msq, &req);

    if (req.mtype == RESPAWN)
    {
      taxi_pid = create_taxi(TRUE);
      replace_taxi_pid(req.mtext, taxi_pid);
    }
    else
    {
      taxi_pid = create_taxi(FALSE);
      add_taxi_pid(taxi_pid);
    }
  }

  return 0;
}
