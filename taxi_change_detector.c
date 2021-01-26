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
void init_taxi_stats();

/* STATISTICHE */
int g_total_travels;                /* viaggi totali (successo, abortiti ecc.) */
int *g_top_cells;             /* posizione celle più attraversate */
TaxiStats g_taxi_most_street;      /* Taxi che ha percorso più celle */
TaxiStats g_taxi_longest_travel; /* Taxi che ha fatto il viaggio più lungo */
TaxiStats g_taxi_most_requests;    /* Taxi che ha raccolto più richieste */

int g_taxi_info_msq_id;
TaxiStatus *g_taxi_status_list;

int main(int argc, char const *argv[])
{
    g_taxi_info_msq_id = read_id_from_file("taxi_info_msq");
    init_taxi_stats();

    while (TRUE)
    {
        update_taxi_status();
    }
    
    return 0;
}

void init_taxi_stats() {
    g_taxi_most_street.crossed_cells = -1;
    g_taxi_most_street.max_travel_time = -1;
    g_taxi_most_street.requests = -1;

    g_taxi_longest_travel.crossed_cells = -1;
    g_taxi_longest_travel.max_travel_time = -1;
    g_taxi_longest_travel.requests = -1;

    g_taxi_most_requests.crossed_cells = -1;
    g_taxi_most_requests.max_travel_time = -1;
    g_taxi_most_requests.requests = -1;
}

void copy_taxi_stats(TaxiStats * src, TaxiStats * dest) {
    dest->crossed_cells = src->crossed_cells;
    dest->max_travel_time = src->max_travel_time;
    dest->requests = src->requests;
}

void update_taxi_stats(TaxiStats taxi_stats) {
    if (taxi_stats.crossed_cells > g_taxi_most_street.crossed_cells) {
        copy_taxi_stats(&taxi_stats, &g_taxi_most_street);
    }

    if (taxi_stats.max_travel_time > g_taxi_longest_travel.max_travel_time) {
        copy_taxi_stats(&taxi_stats, &g_taxi_longest_travel);
    }

    if (taxi_stats.requests > g_taxi_most_requests.requests) {
        copy_taxi_stats(&taxi_stats, &g_taxi_most_requests);
    }
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
    status[i].taxi_stats.crossed_cells = 0;
    status[i].taxi_stats.max_travel_time = 0;
    status[i].taxi_stats.requests = 0;
  }
}

/* Gather taxi status updates */
void update_taxi_status()
{
    TaxiActionMsg msg;
    int err;

    err = msgrcv(g_taxi_info_msq_id, &msg, sizeof msg.mtext, 0, 0);
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

    update_taxi_stats(update.mtext.taxi_stats);

    DEBUG_RAISE_INT(err);
    if (update.mtype == SPAWNED)
    {
        g_taxi_status_list[index].pid = update.mtext.pid;
        g_taxi_status_list[index].available = TRUE;
        g_taxi_status_list[index].position = update.mtext.position;
    }
    else if (update.mtype == PICKUP)
    {
        g_total_travels += 1;
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
