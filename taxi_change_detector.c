#define _GNU_SOURCE

#include "params.h"
#include "data_structures.h"
#include "params.h"
#include "sem_lib.h"
#include "utils.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#define CHANGES_FILE "changes.txt"
#define UNSERVED_FILE "unserved.txt"

void update_taxi_availability_list(TaxiActionMsg update);
void init_taxi_stats();
void update_taxi_status(enum Bool);
void write_update_to_file(TaxiActionMsg msg);
char *get_status_by_id(long op);
char *get_string_by_bool(enum Bool val);
void set_handler();
void taxi_change_detector_handler(int signum);
void create_taxi_availability_list();
void write_unserved_to_file(RequestMsg msg);
void close_files();

FILE *changes_f;
FILE *unserved_f;

/* STATISTICHE */
int g_total_travels;             /* viaggi totali (successo, abortiti ecc.) */
int *g_top_cells;                /* posizione celle più attraversate */
TaxiStats g_taxi_most_street;    /* Taxi che ha percorso più celle */
TaxiStats g_taxi_longest_travel; /* Taxi che ha fatto il viaggio più lungo */
TaxiStats g_taxi_most_requests;  /* Taxi che ha raccolto più richieste */

int g_taxi_info_msq_id;
int g_requests_msq_id;
TaxiStatus *g_taxi_status_list;

int main(int argc, char const *argv[])
{
  set_handler();
  g_taxi_info_msq_id = read_id_from_file("taxi_info_msq");
  g_requests_msq_id = read_id_from_file("requests_msq");
  init_taxi_stats();
  create_taxi_availability_list();
  changes_f = fopen(CHANGES_FILE, "w+");
  CHECK_FILE(changes_f, CHANGES_FILE);
  while (TRUE)
  {
    update_taxi_status(FALSE);
  }

  atexit(close_files);
  return 0;
}

void close_files()
{
  fclose(changes_f);
  fclose(unserved_f);
}

void init_taxi_stats()
{
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

void copy_taxi_stats(TaxiStats *src, TaxiStats *dest)
{
  dest->crossed_cells = src->crossed_cells;
  dest->max_travel_time = src->max_travel_time;
  dest->requests = src->requests;
}

void update_taxi_stats(TaxiStats taxi_stats)
{
  if (taxi_stats.crossed_cells > g_taxi_most_street.crossed_cells)
  {
    copy_taxi_stats(&taxi_stats, &g_taxi_most_street);
  }

  if (taxi_stats.max_travel_time > g_taxi_longest_travel.max_travel_time)
  {
    copy_taxi_stats(&taxi_stats, &g_taxi_longest_travel);
  }

  if (taxi_stats.requests > g_taxi_most_requests.requests)
  {
    copy_taxi_stats(&taxi_stats, &g_taxi_most_requests);
  }
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
    g_taxi_status_list[i].taxi_stats.crossed_cells = 0;
    g_taxi_status_list[i].taxi_stats.max_travel_time = 0;
    g_taxi_status_list[i].taxi_stats.requests = 0;
  }
}

void collect_unserved_requests()
{
  RequestMsg msg;
  int err;
  unserved_f = fopen(UNSERVED_FILE, "w+");
  CHECK_FILE(unserved_f, UNSERVED_FILE);
  while (TRUE)
  {
    err = msgrcv(g_requests_msq_id, &msg, sizeof msg.mtext, 0, IPC_NOWAIT);
    if (errno != ENOMSG)
    {
      DEBUG_RAISE_INT(err);
      write_unserved_to_file(msg);
    }
    else
    {
      break;
    }
  }
}

/* Gather taxi status updates */
void update_taxi_status(enum Bool empty)
{
  TaxiActionMsg msg;
  int err;

  if (empty == TRUE)
  {
    while (TRUE)
    {
      err = msgrcv(g_taxi_info_msq_id, &msg, sizeof msg.mtext, 0, IPC_NOWAIT);
      if (errno != ENOMSG)
      {
        DEBUG_RAISE_INT(err);
        update_taxi_availability_list(msg);
        write_update_to_file(msg);
      }
      else
      {
        errno = 0;
        break;
      }
    }
  }
  else
  {
    err = msgrcv(g_taxi_info_msq_id, &msg, sizeof msg.mtext, 0, 0);
    DEBUG_RAISE_INT(err);

    update_taxi_availability_list(msg);
    write_update_to_file(msg);
  }
}

/* updates taxi status in shared memory */
void update_taxi_availability_list(TaxiActionMsg update)
{
  int err;
  pid_t *current;
  int index = 0;
  while (current == NULL && index < SO_TAXI)
  {
    if (update.mtype == SPAWNED && g_taxi_status_list[index].pid == -1)
    {
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

void write_update_to_file(TaxiActionMsg msg)
{

  fseek(changes_f, 0, SEEK_END);
  fprintf(changes_f, "status=%s, pid=%d, available=%s, position=%d\n",
          get_status_by_id(msg.mtype), msg.mtext.pid,
          get_string_by_bool(msg.mtext.available), msg.mtext.position);
}

void write_unserved_to_file(RequestMsg msg)
{

  fseek(unserved_f, 0, SEEK_END);
  fprintf(unserved_f, "origin=%d, destination=%d\n",
          msg.mtext.origin,
          msg.mtext.destination);
}

char *get_string_by_bool(enum Bool val)
{
  if (val == TRUE)
  {
    return "TRUE";
  }
  else
  {
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

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_change_detector_handler;

  sigaction(SIGTERM, &act, NULL);
}

/* Signal handler del processo taxi_change_detector */
void taxi_change_detector_handler(int signum)
{
  int i, err;
  char selection;

  switch (signum)
  {
  case SIGTERM: /* Interrupts the simulation - politely ask a program to terminate - can be blocked, handled, and ignored */
    update_taxi_status(TRUE);
    collect_unserved_requests();

    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}
