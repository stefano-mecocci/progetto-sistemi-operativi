#define _GNU_SOURCE

#include "params.h"
#include "data_structures.h"
#include "utils.h"

#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int taxi_list_mem_id, taxi_info_msq_id, taxi_request_msq_id;

int main(int argc, char const *argv[])
{
    taxi_list_mem_id = read_id_from_file("taxi_list_id");
    taxi_info_msq_id = read_id_from_file("taxi_info_msq");
    taxi_request_msq_id = read_id_from_file("requests_msq");
    while (TRUE)
    {
        update_taxi_status();
    }
    
    return 0;
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
    TaxiStatus *taxis = shmat(taxi_list_mem_id, NULL, 0);
    pid_t *current;
    int index = 0;
    while (current == NULL && index < SO_TAXI)
    {
        if (taxis[index].pid == update.mtext.pid)
        {
            current = taxis[index].pid;
        }
        index++;
    }

    if (update.mtype == SPAWN)
    {
        taxis[index].pid = update.mtext.pid;
        taxis[index].available = TRUE;
        taxis[index].position = update.mtext.position;
    }
    else if (update.mtype == PICKUP)
    {
        taxis[index].available = FALSE;
    }
    else if (update.mtype == BASICMOV)
    {
        taxis[index].position = update.mtext.position;
    }
    else if (update.mtype == SERVED)
    {
        taxis[index].available = TRUE;
    }
    else if (update.mtype == TIMEOUT)
    {
        taxis[index].pid = -1;
        taxis[index].available = FALSE;
        taxis[index].position = -1;
        dequeue_invalid_requests(update.mtext.pid);
    }
    else if (update.mtype == ABORTED)
    {
        taxis[index].pid = -1;
        taxis[index].available = FALSE;
        taxis[index].position = -1;
        dequeue_invalid_requests(update.mtext.pid);
    }
    else
    {
        /* taxi operation out of known range */
    }

    shmdt(taxis);
}

void dequeue_invalid_requests(pid_t pid){
    Request req;
    int err;
    while (errno != ENOMSG)
    {
        err = msgrcv(taxi_info_msq_id, NULL, sizeof(req), pid, IPC_NOWAIT);        
        DEBUG_RAISE_INT(err);
    }
}
