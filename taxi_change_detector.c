#define _GNU_SOURCE

#include "lib/params.h"
#include "lib/data_structures.h"
#include "lib/params.h"
#include "lib/utils.h"
#include "lib/linked_list.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#define CHANGES_FILE "./out/changes.log"
#define UNSERVED_FILE "./out/unserved.log"
#define REPORT_FILE "./out/report.txt"

void update_taxi_availability_list(TaxiActionMsg update);
void update_taxi_status(enum Bool);
void write_update_to_file(TaxiActionMsg msg);
char *get_status_by_id(long op);
char *get_string_by_bool(enum Bool val);
void set_handler();
void taxi_change_detector_handler(int signum);
void create_taxi_availability_list();
void write_unserved_to_file(RequestMsg msg);
void close_files();
void init_stats();

FILE *changes_f;
FILE *unserved_f;
FILE *report_file;

/* STATISTICHE */
Tuple *g_top_cells = NULL; /* posizione celle più attraversate */

TaxiStats g_most_street = {-1, -1, -1, -1};
TaxiStats g_longest_travel_time = {-1, -1, -1, -1};
TaxiStats g_most_requests = {-1, -1, -1, -1};

int g_successed_requests = 0;
int g_unserved_requests = 0;
int g_aborted_requests = 0;

int *g_crossed_cells_num = NULL;
List g_taxi_pids = NULL;

int g_city_id;
int g_city_sems_cap;
int g_taxi_info_msq_id;
int g_requests_msq_id;
TaxiStatus *g_taxi_status_list;

int main(int argc, char const *argv[])
{
  set_handler();
  g_city_id = read_id_from_file(IPC_CITY_ID_FILE);
  g_city_sems_cap = read_id_from_file(IPC_CITY_SEMS_CAP_FILE);
  g_taxi_info_msq_id = read_id_from_file(IPC_TAXI_INFO_MSQ_FILE);
  g_requests_msq_id = read_id_from_file(IPC_REQUESTS_MSQ_FILE);
  init_stats();
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

void init_stats()
{
  int i;

  g_top_cells = malloc(sizeof(Tuple) * SO_TOP_CELLS);
  DEBUG_RAISE_ADDR(g_top_cells);
  g_crossed_cells_num = malloc(sizeof(int) * SO_WIDTH * SO_HEIGHT);
  DEBUG_RAISE_ADDR(g_crossed_cells_num);

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    g_crossed_cells_num[i] = 0;
  }
}

void close_files()
{
  fclose(changes_f);
  fclose(unserved_f);
  fclose(report_file);
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
    g_taxi_status_list[i].longest_travel_time = -1;
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
  int err = 0;
  pid_t *current = NULL;
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

  DEBUG_RAISE_INT(err);

  if (update.mtype == SPAWNED)
  {
    g_taxi_status_list[index].pid = update.mtext.pid;
    g_taxi_status_list[index].available = TRUE;
    g_taxi_status_list[index].position = update.mtext.position;
    g_taxi_pids = list_add(g_taxi_pids, update.mtext.pid);
  }
  else if (update.mtype == DEQUEUE)
  {
    list_increase_taxi_requests(g_taxi_pids, update.mtext.pid);
    g_taxi_status_list[index].available = FALSE;
  }
  else if (update.mtype == PICKUP)
  {
    g_taxi_status_list[index].available = FALSE;
  }
  else if (update.mtype == BASICMOV)
  {
    g_crossed_cells_num[update.mtext.position]++;
    list_increase_taxi_crossed_cells(g_taxi_pids, update.mtext.pid);
    g_taxi_status_list[index].position = update.mtext.position;
  }
  else if (update.mtype == SERVED)
  {
    if (update.mtext.longest_travel_time > g_longest_travel_time.longest_travel_time)
    {
      g_longest_travel_time.longest_travel_time = update.mtext.longest_travel_time;
      g_longest_travel_time.pid = update.mtext.pid;
    }

    g_successed_requests += 1;
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
  char *status;
  if (msg.mtype == NORMAL)
  {
    status = "UNSERVED";
    g_unserved_requests++;
  }
  else
  {
    status = "ABORTED";
    g_aborted_requests++;
  }
  fseek(unserved_f, 0, SEEK_END);
  fprintf(unserved_f, "origin=%d, destination=%d - %s\n",
          msg.mtext.origin,
          msg.mtext.destination,
          status);
}

char *get_string_by_bool(enum Bool val)
{
  if (val)
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
  if (op == SPAWNED)
  {
    return "SPAWNED";
  }
  else if (op == DEQUEUE)
  {
    return "DEQUEUE";
  }
  else if (op == PICKUP)
  {
    return "PICKUP";
  }
  else if (op == BASICMOV)
  {
    return "BASICMOV";
  }
  else if (op == SERVED)
  {
    return "SERVED";
  }
  else if (op == TIMEOUT)
  {
    return "TIMEOUT";
  }
  else if (op == ABORTED)
  {
    return "ABORTED";
  }
  else
  {
    /* taxi operation out of known range */
    return "INVALID STATUS";
  }
}

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_change_detector_handler;

  sigaction(SIGTERM, &act, NULL);
}

Tuple find_highest_cell()
{
  Tuple res = {-1, -1};
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    if (g_crossed_cells_num[i] > res.value)
    {
      res.key = i;
      res.value = g_crossed_cells_num[i];
    }
  }

  g_crossed_cells_num[res.key] = -1;

  return res;
}

void calc_top_cells()
{
  int i = 0;

  for (i = 0; i < SO_TOP_CELLS; i++)
  {
    g_top_cells[i] = find_highest_cell();
  }
}

int get_top_cell_value(int index)
{
  int i = 0, found = 0;
  while (found == 0 && i < SO_TOP_CELLS)
  {
    if (g_top_cells[i].key == index)
    {
      found++;
    }
    else
    {
      i++;
    }
  }

  return found > 0 ? g_top_cells[i].value : -1;
}

void copy_taxi_stats(const TaxiStats *src, TaxiStats *dest)
{
  dest->crossed_cells = src->crossed_cells;
  dest->longest_travel_time = src->longest_travel_time;
  dest->requests = src->requests;
  dest->pid = src->pid;
}

void calc_taxi_stats()
{
  List p = g_taxi_pids;

  while (p != NULL)
  {
    if (p->taxi_stats.crossed_cells > g_most_street.crossed_cells)
    {
      copy_taxi_stats(&(p->taxi_stats), &g_most_street);
    }

    if (p->taxi_stats.requests > g_most_requests.requests)
    {
      copy_taxi_stats(&(p->taxi_stats), &g_most_requests);
    }

    if (p->taxi_stats.longest_travel_time > g_longest_travel_time.longest_travel_time)
    {
      copy_taxi_stats(&(p->taxi_stats), &g_longest_travel_time);
    }

    p = p->next;
  }
}

void calc_stats()
{
  calc_top_cells();
  calc_taxi_stats();
}

void fprint_taxi_time(FILE *report_file, TaxiStats taxi)
{
  fprintf(report_file, "- Pid = %d\n", taxi.pid);
  fprintf(report_file, "- Longest travel = %ldms\n\n", taxi.longest_travel_time);
}

void fprint_taxi_requests(FILE *report_file, TaxiStats taxi)
{
  fprintf(report_file, "- Pid = %d\n", taxi.pid);
  fprintf(report_file, "- Requests = %d\n\n", taxi.requests);
}

void fprint_taxi_cells(FILE *report_file, TaxiStats taxi)
{
  fprintf(report_file, "- Pid = %d\n", taxi.pid);
  fprintf(report_file, "- Crossed cells = %d\n\n", taxi.crossed_cells);
}

#define SEPARATOR "\n--------------------------------------------------\n"

void write_stats_to_report_file()
{
  int i;
  Point p;

  report_file = fopen(REPORT_FILE, "w+");
  CHECK_FILE(report_file, REPORT_FILE)

  fprintf(report_file, SEPARATOR);
  fprintf(report_file, "\nSucceded requests: %d\n", g_successed_requests);
  fprintf(report_file, "Unserved requests: %d\n", g_unserved_requests);
  fprintf(report_file, "Aborted requests: %d\n", g_aborted_requests);

  fprintf(report_file, SEPARATOR);
  fprintf(report_file, "Top cells: (x, y) - # \n");
  for (i = 0; i < SO_TOP_CELLS; i++)
  {
    p = index2point(g_top_cells[i].key);
    fprintf(report_file, "(%d, %d) - %d\n", p.x, p.y, g_top_cells[i].value);
  }

  print_city(report_file, g_city_id, g_city_sems_cap, TOP_CELLS, get_top_cell_value);

  fprintf(report_file, SEPARATOR);

  fprintf(report_file, "Sources:\n");
  print_city(report_file, g_city_id, g_city_sems_cap, SOURCES, get_top_cell_value);

  fprintf(report_file, SEPARATOR);

  fprintf(report_file, "Taxi who travelled most (distance):\n");
  fprint_taxi_cells(report_file, g_most_street);
  fprintf(report_file, "Taxi who travelled most (time):\n");
  fprint_taxi_time(report_file, g_longest_travel_time);
  fprintf(report_file, "Taxi who took more requests:\n");
  fprint_taxi_requests(report_file, g_most_requests);
}

/* Signal handler del processo taxi_change_detector */
void taxi_change_detector_handler(int signum)
{
  switch (signum)
  {
  case SIGTERM: /* Interrupts the simulation - politely ask a program to terminate - can be blocked, handled, and ignored */
    update_taxi_status(TRUE);
    collect_unserved_requests();
    calc_stats();
    write_stats_to_report_file();
    printf("Stats generated in %s file\n", REPORT_FILE);

    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}
