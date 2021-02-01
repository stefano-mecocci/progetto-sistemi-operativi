#define _GNU_SOURCE

#include "taxi_lib.h"
#include "data_structures.h"
#include "params.h"
#include "utils.h"

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
#include "astar/pathfinder.h"

#define TAXIPIDS_SIZE (SO_TAXI + 1)

clockid_t g_stopwatch;

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
int g_pos;

/* A* stuff */
NodeDataMap *g_dataMap = NULL;


void taxi_handler(int, siginfo_t *, void *);
void send_spawn_request();
void insert_aborted_request();
void sem_transfer_capacity(int next_addr);

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
}

void copy_city()
{
  int i;
  g_city = malloc(sizeof(Cell) * (SO_WIDTH * SO_HEIGHT));
  DEBUG_RAISE_ADDR(g_city);
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
  TaxiStatus status;
  int err;
  err = msgrcv(g_requests_msq, req, sizeof req->mtext, FAILED, MSG_EXCEPT);
  DEBUG_RAISE_INT(g_master_pid, err);
  g_last_request = req;

  status.available = FALSE;
  status.pid = getpid();
  status.position = get_position();
  send_taxi_update(g_taxi_info_msq, DEQUEUE, status);
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

    free(g_dataMap);
    free(g_city);

    exit(EXIT_TIMER);

    break;

  case SIGTERM:
    send_taxi_update(g_taxi_info_msq, ABORTED, status);

    if (g_serving_req == TRUE)
    {
      insert_aborted_request();
    }
    free(g_dataMap);
    free(g_city);

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

int CustomGetMap( int x, int y )
{
	if( x < 0 ||
	    x >= SO_WIDTH ||
		 y < 0 ||
		 y >= SO_HEIGHT
	  )
	{
		return 9;	 
	}

  int index = coordinates2index(x, y);
  enum cell_type type = g_city[index].type;

	return type == CELL_HOLE ? 9 : 1;
}

float CostOfGoal(int X1,int Y1, int X2, int Y2,int (*GetMap)(int,int))
{
  return sqrt((float)((X2-X1)*(X2-X1))+((Y2-Y1)*(Y2-Y1)));
}

void init_astar()
{
  int i;
  if (g_dataMap != NULL)
  {
    free(g_dataMap);
  }
  g_dataMap = malloc(sizeof(*g_dataMap) * SO_WIDTH * SO_HEIGHT);

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    g_dataMap[i].GScore   = 0.0;
    g_dataMap[i].FScore   = 0.0;
    g_dataMap[i].CameFrom = NULL;
  }
}

AStar_Node *get_path(int position, int destination)
{
  AStar_Node *Solution;
  AStar_Node *NextInSolution;
  AStar_Node *SolutionNavigator;
  Point start = index2point(position);
  Point end = index2point(destination);
  int index;

  Solution = AStar_Find(SO_WIDTH, SO_HEIGHT, start.x, start.y, end.x, end.y, CustomGetMap, g_dataMap);
  if (!Solution)
  {
    printf("[taxi](%d) No solution was found for start=%d; end=%d\n", getpid(), position, destination);
    raise(SIGALRM);
  }
  SolutionNavigator = NULL;
  NextInSolution = Solution;
  
  /* Reverse the solution */
  if (NextInSolution)
  {
    do
    {
      index = coordinates2index(NextInSolution->X, NextInSolution->Y);

      NextInSolution->NextInSolvedPath = SolutionNavigator;
      SolutionNavigator = NextInSolution;
      NextInSolution = g_dataMap[index].CameFrom;
    }
    while ((SolutionNavigator->X != start.x) || (SolutionNavigator->Y != start.y));
  }
  return SolutionNavigator;
}

void travel(AStar_Node *navigator)
{
  int i, crossing_time, next_addr;
  TaxiStatus status;
  Point p = index2point(get_position());
  while (navigator = navigator->NextInSolvedPath)
  {
    next_addr = coordinates2index(navigator->X, navigator->Y);

    crossing_time = g_city[next_addr].cross_time;

    sem_transfer_capacity(next_addr);

    start_timer();

    set_position(next_addr);
    sleep_for(0, crossing_time);

    status.available = FALSE;
    status.pid = getpid();
    status.position = get_position();

    send_taxi_update(g_taxi_info_msq, BASICMOV, status);
  }
}

void sem_transfer_capacity(int next_addr)
{
  struct sembuf ops[2];

  ops[0].sem_flg = 0;
  ops[0].sem_num = next_addr;
  ops[0].sem_op = -1;

  ops[1].sem_flg = 0;
  ops[1].sem_num = get_position();
  ops[1].sem_op = 1;

  semop(g_city_sems_cap, ops, 2);
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
