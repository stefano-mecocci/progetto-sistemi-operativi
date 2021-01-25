#include "params.h"
#include "data_structures.h"
#include "utils.h"
#include "sem_lib.h"

#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>

void update_taxi_availability_list(TaxiActionMsg update);
void update_taxi_status();

int taxi_info_msq_id;
TaxiStatus *g_taxi_status_list;

int main(int argc, char const *argv[])
{
    taxi_info_msq_id = read_id_from_file("taxi_info_msq");
    while (TRUE)
    {
        update_taxi_status();
    }
    
    return 0;
}

void create_taxi_availability_list()
{
  int i;
  TaxiStatus *status;
  g_taxi_status_list = calloc(SO_TAXI, sizeof(TaxiStatus));

  for (i = 0; i < SO_TAXI; i++)
  {
    status[i].pid = -1;
    status[i].available = FALSE;
    status[i].position = -1;
  }
}

/* Gather taxi status updates */
void update_taxi_status()
{
    TaxiActionMsg msg;
    int err;

    err = msgrcv(taxi_info_msq_id, &msg, sizeof msg.mtext, 0, 0);
    DEBUG_RAISE_INT(err);

    update_taxi_availability_list(msg);
}

/* updates taxi status in shared memory */
void update_taxi_availability_list(TaxiActionMsg update)
{
    int err;
    pid_t *current;
    int index = 0;
    while (current == NULL && index < SO_TAXI)
    {
        if (update.mtype == SPAWNED || g_taxi_status_list[index].pid == update.mtext.pid)
        {
            current = &(g_taxi_status_list[index].pid);
        }
        else
        {
            index++;
        }
    }

    DEBUG_RAISE_INT(err);
    if (update.mtype == SPAWNED)
    {
        g_taxi_status_list[index].pid = update.mtext.pid;
        g_taxi_status_list[index].available = TRUE;
        g_taxi_status_list[index].position = update.mtext.position;
    }
    else if (update.mtype == PICKUP)
    {
        g_taxi_status_list[index].available = FALSE;
    }
    else if (update.mtype == BASICMOV)
    {
        g_taxi_status_list[index].position = update.mtext.position;
    }
    else if (update.mtype == SERVED)
    {
        g_taxi_status_list[index].available = TRUE;
    }
    else if (update.mtype == TIMEOUT)
    {
        g_taxi_status_list[index].pid = -1;
        g_taxi_status_list[index].available = FALSE;
        g_taxi_status_list[index].position = -1;
    }
    else if (update.mtype == ABORTED)
    {
        g_taxi_status_list[index].pid = -1;
        g_taxi_status_list[index].available = FALSE;
        g_taxi_status_list[index].position = -1;
    }
    else
    {
        /* taxi operation out of known range */
    }
}
