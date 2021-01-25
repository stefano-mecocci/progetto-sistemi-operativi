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

#define FILENAME "changes.txt"

void update_taxi_availability_list(TaxiActionMsg update);
void update_taxi_status();
void write_update_to_file(TaxiActionMsg msg);
char *get_status_by_id(long op);
char *get_string_by_bool(enum Bool val);
FILE *file;

int taxi_info_msq_id;
TaxiStatus *g_taxi_status_list;

int main(int argc, char const *argv[])
{
    create_taxi_availability_list();
    taxi_info_msq_id = read_id_from_file("taxi_info_msq");
    file = fopen(FILENAME, "w+");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    while (TRUE)
    {
        update_taxi_status();
    }

    atexit(fclose(file));
    return 0;
}

void create_taxi_availability_list()
{
    int i;
    g_taxi_status_list = calloc(SO_TAXI, sizeof(TaxiStatus));

    for (i = 0; i < SO_TAXI; i++)
    {
        g_taxi_status_list[i].pid = -1;
        g_taxi_status_list[i].available = FALSE;
        g_taxi_status_list[i].position = -1;
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
    write_update_to_file(msg);
}

/* updates taxi status in shared memory */
void update_taxi_availability_list(TaxiActionMsg update)
{
    int err;
    pid_t *current;
    int index = 0;
    while (current == NULL && index < SO_TAXI)
    {
        if(update.mtype == SPAWNED && g_taxi_status_list[index].pid == -1){
            current = &update.mtext.pid;
        }
        else if (g_taxi_status_list[index].pid == update.mtext.pid)
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

void write_update_to_file(TaxiActionMsg msg)
{
    
    fseek(file, 0, SEEK_END);
    fprintf(file, "status=%s, pid=%d, available=%s, position=%d\n",
            get_status_by_id(msg.mtype),
            msg.mtext.pid,
            get_string_by_bool(msg.mtext.available),
            msg.mtext.position);
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
