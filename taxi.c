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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TAXIPIDS_SIZE (SO_TAXI + 1)

int g_taxi_spawn_msq;
int g_taxi_info_msq;
int g_sync_sems;
int g_city_id;
int g_city_sems_cap;
int g_requests_msq;
City g_city;

enum Bool g_serving_req = FALSE;
RequestMsg *g_last_request;
pid_t g_master_pid;
TaxiStats g_data;
int g_pos;
astar_t *g_as;

void taxi_handler(int, siginfo_t *, void *);
void send_spawn_request();
void print_path(direction_t *directions, int steps);
void insert_aborted_request();

/*
====================================
  PUBLIC
====================================
*/

void init_data_ipc(int taxi_spawn_msq, int taxi_info_msq, int sync_sems, int city_id, int city_sems_cap, int requests_msq)
{
  g_taxi_spawn_msq = taxi_spawn_msq;
  g_taxi_info_msq = taxi_info_msq;
  g_sync_sems = sync_sems;
  g_city_id = city_id;
  g_city_sems_cap = city_sems_cap;
  g_requests_msq = requests_msq;
}

void init_data(int master_pid, int pos)
{
  g_master_pid = master_pid;
  g_pos = pos;

  g_data.crossed_cells = 0;
  g_data.max_travel_time = 0;
  g_data.requests = 0;
}

void copy_city(){
  int i;
  g_city = malloc(sizeof(Cell)*(SO_WIDTH * SO_HEIGHT));
  City city = shmat(g_city_id, NULL, SHM_RDONLY);
  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    g_city[i].type = city[i].type;
    g_city[i].cross_time = city[i].cross_time;
    g_city[i].capacity = city[i].capacity;
  }
  shmdt(city);
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
  alarm(SO_TIMEOUT);
}

void receive_ride_request(RequestMsg *req)
{
  int err;
  err = msgrcv(g_requests_msq, req, sizeof req->mtext, FAILED, MSG_EXCEPT);
  DEBUG_RAISE_INT(g_master_pid, err);
  g_last_request = req;
}

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_sigaction = taxi_handler;
  act.sa_flags = SA_NODEFER | SA_SIGINFO;

  sigaction(SIGALRM, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
}

/*
====================================
  PRIVATE
====================================
*/

void taxi_handler(int signum, siginfo_t *info, void *context)
{
  int i, err, timer_status;
  TaxiStatus status;
  status.pid = getpid();
  status.position = g_pos;

  switch (signum)
  {
  case SIGALRM:
    err = send_taxi_update(g_taxi_info_msq, TIMEOUT, status);
    DEBUG_RAISE_INT(err);
    send_spawn_request(); 
    if (g_serving_req == TRUE)
    {
      insert_aborted_request();
    }
    exit(EXIT_TIMER);

    break;

  case SIGTERM:
    printf("[taxi](%d) ABORTING\n", getpid());
    send_taxi_update(g_taxi_info_msq, ABORTED, status);

    if (g_serving_req == TRUE)
    {
      insert_aborted_request();
    }
    exit(EXIT_SUCCESS);
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
  enum cell_type type = g_city[index].type;
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

void copy_taxi_stats(TaxiStats *src, TaxiStats *dest)
{
  dest->crossed_cells = src->crossed_cells;
  dest->max_travel_time = src->max_travel_time;
  dest->requests = src->requests;
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
    crossing_time = g_city[next_addr].cross_time;

    sem_reserve(g_city_sems_cap, next_addr);
    sem_release(g_city_sems_cap, get_position());

    start_timer();

    set_position(next_addr);
    sleep_for(0, crossing_time);

    status.available = FALSE;
    status.pid = getpid();
    status.position = get_position();

    g_data.crossed_cells += 1;
    g_data.max_travel_time = 10; /* TODO: calc the travel time */
    g_data.requests += 0;        /* TODO: add 1 if requets taken */
    copy_taxi_stats(&g_data, &status.taxi_stats);

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
           p.y);
  }
}

void set_aborted_request(enum Bool serving)
{
  g_serving_req = serving;
}

void insert_aborted_request()
{
  RequestMsg req;
  if (g_last_request != NULL)
  {
    req.mtype = (int)FAILED;
    req.mtext.origin = g_last_request->mtext.origin;
    req.mtext.destination = g_last_request->mtext.destination;
    int err = msgsnd(g_requests_msq, &req, sizeof(Ride), 0);
    DEBUG_RAISE_INT(getppid(), err);
  }
}
