#define _GNU_SOURCE

#include "taxi.h"
#include "data_structures.h"
#include "params.h"
#include "utils.h"
#include "sem_lib.h"
#include "astar/astar.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TAXIPIDS_SIZE (SO_TAXI + 1)

int g_taxi_spawn_msq;
int g_taxi_info_msq;
int g_sync_sems;
int g_city_id;
int g_city_sems_cap;

pid_t g_master_pid, g_timer_pid;
TaxiStats g_data;
int g_pos;
astar_t *g_as;

void taxi_handler(int signum);
void send_spawn_request();
void send_taxi_data();
void print_path(direction_t *directions, int steps);

/*
====================================
  PUBLIC
====================================
*/

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = taxi_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGCONT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems, int city_id, int city_sems_cap)
{
  g_taxi_spawn_msq = taxi_spawn_msq;
  g_taxi_info_msq = taxi_info_msq;
  g_sync_sems = sync_sems;
  g_city_id = city_id;
  g_city_sems_cap = city_sems_cap;
}

void init_data(int master_pid, int pos)
{
  g_master_pid = master_pid;
  g_pos = pos;

  g_data.crossed_cells = 1;
  g_data.max_travel_time = 2;
  g_data.requests = 3;
}

int get_position()
{
  return g_pos;
}

void set_position(int addr)
{
  g_pos = addr;
}

void start_timer()
{
  pid_t pid = fork();
  char *args[2] = {"taxi_timer.o", NULL};
  int err;

  DEBUG_RAISE_INT(pid);

  if (pid == 0)
  {
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      kill(getppid(), SIGTERM);
      exit(EXIT_ERROR);
    }
  }
  else
  {
    g_timer_pid = pid;
  }
}

void receive_ride_request(int requests_msq, RequestMsg *req)
{
  int err;
  err = msgrcv(requests_msq, req, sizeof req->mtext, 0, 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}

/*
====================================
  PRIVATE
====================================
*/

/* DEPRECATED(use send_taxi_update instead) - Invia i dati del taxi a master */
void send_taxi_data()
{
  TaxiInfo info;
  int err;

  info.mtype = 1;
  info.mtext[0] = g_data.crossed_cells;
  info.mtext[1] = g_data.max_travel_time;
  info.mtext[2] = g_data.requests;

  err = msgsnd(g_taxi_info_msq, &info, sizeof info.mtext, 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}

void taxi_handler(int signum)
{
  int i, err;
  TaxiStatus status;
  status.pid = getpid();
  status.position = g_pos;

  switch (signum)
  {
  case SIGINT:
    kill(g_timer_pid, SIGSTOP);
    raise(SIGSTOP);
    break;

  case SIGCONT:
    kill(g_timer_pid, SIGCONT);
    break;

  case SIGTERM:

    exit(EXIT_ERROR);
    break;

  case SIGUSR1: /* TIMEOUT */
    block_signal(SIGUSR2);

    /* send_taxi_data(); */
    send_taxi_update(g_taxi_info_msq, TIMEOUT, status);
    send_spawn_request();
    err = sem_op(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);
    DEBUG_RAISE_INT(g_master_pid, err);

    unblock_signal(SIGUSR2);
    exit(EXIT_TIMER);
    break;

  case SIGUSR2: /* END OF SIMULATION */
    send_taxi_update(g_taxi_info_msq, ABORTED, status);
    err = sem_op(g_sync_sems, SEM_ALIVES_TAXI, -1, 0);
    DEBUG_RAISE_INT(g_master_pid, err);

    exit(EXIT_TIMER);
    break;
  }
}

/* Invia una richiesta di spawn a taxigen */
void send_spawn_request()
{
  SpawnMsg req;
  int err;

  req.mtype = RESPAWN;
  req.mtext[0] = getpid();
  req.mtext[1] = g_pos;

  err = msgsnd(g_taxi_spawn_msq, &req, sizeof req.mtext, 0);
  DEBUG_RAISE_INT(g_master_pid, err);
}

uint8_t get_map_cost(const uint32_t x, const uint32_t y)
{
  int index = coordinates2index(x, y);
  enum cell_type type = get_cell_type(g_city_id, index);
  uint8_t cost = type == CELL_HOLE ? COST_BLOCKED : 1;
  return cost;
}

void init_astar()
{
  /* Allocate an A* context. */
  g_as = astar_new(SO_WIDTH, SO_HEIGHT, get_map_cost, NULL);
  astar_set_origin(g_as, 0, 0);
  astar_set_movement_mode(g_as, DIR_CARDINAL);
}

direction_t *get_path(int position, int destination, int *steps)
{
  direction_t *directions;
  Point start = index2point(position);
  Point end = index2point(destination);

  /* Look for a route. */
  int result = astar_run(g_as, start.x, start.y, end.x, end.y);
  if (astar_have_route(g_as))
  {
    *steps = astar_get_directions(g_as, &directions);
    DEBUG_RAISE_INT(result);
  }
  return directions;
}

void travel(direction_t *directions, int steps)
{
  int i, x, y, crossing_time, next_addr;
  TaxiStatus status;
  Point p = index2point(get_position());
  direction_t *dir = directions;
  for (i = 0; i < steps; i++, dir++)
  {
    x = p.x;
    y = p.y;
    /* Get the next (x, y) co-ordinates of the map. */
    p.x += astar_get_dx(g_as, *dir);
    p.y += astar_get_dy(g_as, *dir);
    next_addr = point2index(p);
    /* printf("step %d: from %d (%d, %d) to %d (%d, %d)\n", 
      i + 1, 
      get_position(),
      x, 
      y,
      next_addr,
      p.x, 
      p.y
    ); */
    crossing_time = get_cell_crossing_time(g_city_id, next_addr);

    sem_reserve(g_city_sems_cap, next_addr);
    sem_release(g_city_sems_cap, get_position());

    set_position(next_addr);
    printf("wait for %f\n", (float)crossing_time/(float)1000000000);
    sleep_for(0, crossing_time);

    status.available = FALSE;
    status.pid = getpid();
    status.position = get_position();
    send_taxi_update(g_taxi_info_msq, BASICMOV, status);
  }

  astar_free_directions(directions);
}


void print_path(direction_t *directions, int steps)
{
  int i, x, y, crossing_time, next_addr;
  TaxiStatus status;
  Point p = index2point(get_position());
  for (i = 0; i < steps; i++, directions++)
  {
    x = p.x;
    y = p.y;
    /* Get the next (x, y) co-ordinates of the map. */
    p.x += astar_get_dx(g_as, *directions);
    p.y += astar_get_dy(g_as, *directions);
    next_addr = point2index(p);
    printf("step %d: %d -> %d ; (%d, %d) -> (%d, %d)\n", 
      i + 1, 
      get_position(),
      next_addr,
      x, 
      y,
      p.x, 
      p.y
    );
  }
}