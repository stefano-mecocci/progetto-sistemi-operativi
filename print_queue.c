#include "data_structures.h"
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

char *get_status_by_id(long op);
char *get_string_by_bool(enum Bool val);
void print_queue_taxi_info(int msqid);

int main(int argc, char const *argv[])
{
    int taxi_info_msq_id = read_id_from_file("taxi_info_msq");
    print_queue_taxi_info(taxi_info_msq_id);

    return 0;
}

void print_queue_taxi_info(int msqid)
{
    TaxiActionMsg msg;
    printf("Unprocessed taxi status changes: \n");
    while (TRUE)
    {
        msgrcv(msqid, &msg, sizeof(TaxiActionMsg), 0, IPC_NOWAIT);
        if (errno != ENOMSG)
        {
            printf("status=%s, pid=%d, available=%s, position=%d\n",
            get_status_by_id(msg.mtype), 
            msg.mtext.pid, 
            get_string_by_bool(msg.mtext.available), 
            msg.mtext.position);
        }
        else
        {
            break;
        }
    }
}

char *get_string_by_bool(enum Bool val){
    if(val == TRUE){
        return "TRUE";
    } else {
        return "FALSE";
    }
}

char *get_status_by_id(long op)
{
    switch (op)
    {
    case 1:
        return "SPAWNED";
        break;
    case 2:
        return "PICKUP";
        break;
    case 3:
        return "BASICMOV";
        break;
    case 4:
        return "SERVED";
        break;
    case 5:
        return "TIMEOUT";
        break;
    case 6:
        return "ABORTED";
        break;
    }
}
