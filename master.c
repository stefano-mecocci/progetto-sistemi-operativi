#define _GNU_SOURCE

#include "master.h"
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
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>


#define ADJACENT_CELLS_NUM 8

/* OGGETTI IPC */
int g_city_id;        /* città */
int g_sync_sems;      /* semafori di sync */
int g_city_sems_op;   /* semafori per operare su celle */
int g_city_sems_cap;  /* semafori per controllare capacità */
int g_requests_msq;   /* Coda richieste */
int g_taxi_info_msq;  /* Coda informazioni statistiche */
int g_taxi_spawn_msq; /* Coda spawns */

/* ENTITÀ */
pid_t *g_source_pids;
pid_t g_taxigen_pid;
pid_t g_mastertimer_pid;
pid_t g_changedetector_pid;
int *g_sources_positions;

void init_taxi(TaxiStats *taxi);
void master_handler(int signum);
void clear_memory();
void generate_adjacent_list(Point p, int list[]);
int is_valid_hole_point(Point p, City city);
void place_hole(int pos, City city);
void create_source(int position);
int generate_origin_point(int, int);
void update_taxi_stats(int taxi_msg[]);
void copy_taxi_data(int taxi_msg[], TaxiStats *taxi);
void send_signal_to_taxigen(int signal);
void send_signal_to_changedetector(int signal);
void send_signal_to_mastertimer(int signal);
void send_signal_to_sources(int signal);

/*
====================================
  FUNZIONI "PUBBLICHE"
====================================
*/

int create_city()
{
  int size = sizeof(Cell) * SO_WIDTH * SO_HEIGHT;
  int id = shmget(IPC_PRIVATE, size, 0660 | IPC_CREAT);
  g_city_id = id;

  DEBUG_RAISE_INT(id);

  return id;
}

int create_sync_sems()
{
  int nsems = 3;
  int id = semget(IPC_PRIVATE, nsems, 0660 | IPC_CREAT);
  g_sync_sems = id;

  DEBUG_RAISE_INT(id);

  return id;
}

int create_city_sems_op()
{
  int nsems = SO_WIDTH * SO_HEIGHT;
  int id = semget(IPC_PRIVATE, nsems, 0660 | IPC_CREAT);
  g_city_sems_op = id;

  DEBUG_RAISE_INT(id);

  return id;
}

int create_city_sems_cap()
{
  int nsems = SO_WIDTH * SO_HEIGHT;
  int id = semget(IPC_PRIVATE, nsems, 0660 | IPC_CREAT);
  g_city_sems_cap = id;

  DEBUG_RAISE_INT(id);

  return id;
}

int create_requests_msq()
{
  int id = msgget(IPC_PRIVATE, 0660 | IPC_CREAT);
  g_requests_msq = id;

  DEBUG_RAISE_INT(id);

  return id;
}

int create_taxi_info_msq()
{
  int id = msgget(IPC_PRIVATE, 0660 | IPC_CREAT);
  DEBUG_RAISE_INT(id);
  g_taxi_info_msq = id;

  return id;
}

int create_taxi_spawn_msq()
{
  int id = msgget(IPC_PRIVATE, 0660 | IPC_CREAT);
  g_taxi_spawn_msq = id;

  DEBUG_RAISE_INT(id);

  return id;
}

void check_params()
{
  if (SO_SOURCES > (SO_WIDTH * SO_HEIGHT - SO_HOLES))
  {
    printf("Errore: troppe sorgenti, città troppo piccola o troppe buche\n");
    raise(SIGTERM);
  }

  if (SO_CAP_MIN > SO_CAP_MAX)
  {
    printf("Errore: SO_CAP_MIN maggiore di SO_CAP_MAX\n");
    raise(SIGTERM);
  }

  if (SO_TIMENSEC_MIN > SO_TIMENSEC_MAX)
  {
    printf("Errore: SO_TIMENSEC_MIN maggiore di SO_TIMENSEC_MAX\n");
    raise(SIGTERM);
  }

  if (SO_TIMEOUT >= SO_DURATION)
  {
    printf("Errore: SO_TIMEOUT maggiore di SO_DURATION\n");
    raise(SIGTERM);
  }
}

void init_data()
{
  int i;

  g_source_pids = malloc(sizeof(pid_t) * SO_SOURCES);
  g_taxigen_pid = -1;
  g_sources_positions = malloc(sizeof(int) * SO_SOURCES);

  DEBUG_RAISE_ADDR(g_source_pids);
  DEBUG_RAISE_ADDR(g_sources_positions);

  for (i = 0; i < SO_SOURCES; i++)
  {
    g_source_pids[i] = -1;
    g_sources_positions[i] = -1;
  }
}

void set_handler()
{
  struct sigaction act;
  bzero(&act, sizeof act);

  act.sa_handler = master_handler;

  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void init_city_cells(int city_id)
{
  City city = shmat(city_id, NULL, 0);
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    city[i].type = CELL_NORMAL;
    city[i].capacity = rand_int(SO_CAP_MIN, SO_CAP_MAX);
    city[i].act_capacity = city[i].capacity;
    city[i].cross_time = rand_int(SO_TIMENSEC_MIN, SO_TIMENSEC_MAX);
    city[i].crossing_num = 0;
  }

  shmdt(city);
}

void init_sync_sems(int sync_sems)
{
  int err = 0;

  err += semctl(sync_sems, SEM_SYNC_TAXI, SETVAL, SO_TAXI);
  err += semctl(sync_sems, SEM_SYNC_SOURCES, SETVAL, SO_SOURCES);

  DEBUG_RAISE_INT(err);
}

void init_city_sems_op(int city_sems_op)
{
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    semctl(city_sems_op, i, SETVAL, 1);
  }
}

void init_city_sems_cap(int city_id, int city_sems_cap)
{
  City city = shmat(city_id, NULL, 0);
  int i;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    if (city[i].type == CELL_NORMAL)
    {
      semctl(city_sems_cap, i, SETVAL, city[i].capacity);
    }
  }

  shmdt(city);
}

void place_city_holes(int city_id)
{
  City city = shmat(city_id, NULL, 0);
  int i = 0, pos = -1;

	srand(time(NULL));

  while (i < SO_HOLES)
  {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    if (is_valid_hole_point(index2point(pos), city))
    {
      place_hole(pos, city);
      i++;
    }
  }

  shmdt(city);
}

void create_taxigen()
{
  pid_t pid = fork();
  char *args[2] = {"taxigen.o", NULL};
  int err;

  DEBUG_RAISE_INT(pid);
  if (pid == 0)
  {
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      raise(SIGTERM);
    }
  }

  g_taxigen_pid = pid;
}

void create_taxis(int taxi_spawn_msq)
{
  int i, err;
  SpawnMsg req;

  for (i = 0; i < SO_TAXI; i++)
  {
    req.mtype = SPAWN;
    req.mtext[0] = -1;
    req.mtext[1] = -1;

    err = msgsnd(taxi_spawn_msq, &req, sizeof req.mtext, 0);
    DEBUG_RAISE_INT(err);
  }
}

void create_sources()
{
  pid_t pid;
  int i, source_point;

  for (i = 0; i < SO_SOURCES; i++)
  {
    source_point = generate_origin_point(g_city_id, g_city_sems_op);
    pid = fork();

    DEBUG_RAISE_INT(pid);

    if (pid == 0)
    {
      create_source(source_point);
    }
    else
    {
      g_sources_positions[i] = source_point;
      g_source_pids[i] = pid;
    }
  }
}

void start_timer()
{
  pid_t timer_pid = fork();
  char *args[2] = {"master_timer.o", NULL};
  int err;

  DEBUG_RAISE_INT(timer_pid);

  if (timer_pid == 0)
  {
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      kill(getppid(), SIGTERM);
      exit(EXIT_FAILURE);
    }
  }
  else
  {

    g_mastertimer_pid = timer_pid;
  }
}

void start_change_detector()
{
  pid_t change_detector_pid = fork();
  char *args[2] = {"taxi_change_detector.o", NULL};
  int err;

  DEBUG_RAISE_INT(change_detector_pid);

  if (change_detector_pid == 0)
  {
    err = execve(args[0], args, environ);

    if (err == -1)
    {
      DEBUG;
      kill(getppid(), SIGTERM);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    g_changedetector_pid = change_detector_pid;
  }
}

void print_city(int city_id)
{
  City city = shmat(city_id, NULL, 0);
  int i, taxi_num;

  for (i = 0; i < SO_WIDTH * SO_HEIGHT; i++)
  {
    if (i % SO_WIDTH == 0)
    {
      printf("\n");
    }
    if (city[i].type == CELL_HOLE)
      {
        printf("x ");
      }
      else
      {
        taxi_num = city[i].capacity - semctl(g_city_sems_cap, i, GETVAL);

        if (taxi_num == 0)
        {
          printf(". ");
        }
        else
        {
          printf("%d ", taxi_num);
        }
      }
  }

  printf("\n\n");
  shmdt(city);
}

/*
====================================
  FUNZIONI "PRIVATE"
====================================
*/

/* Pulisce la memoria dagli oggetti IPC usati e non solo */
void clear_memory()
{
  int err = 0, i;
  
  err = shmctl(g_city_id, IPC_RMID, NULL);
  DEBUG_RAISE_INT(err);
  err = semctl(g_sync_sems, -1, IPC_RMID);
  DEBUG_RAISE_INT(err);
  err = semctl(g_city_sems_op, -1, IPC_RMID);
  DEBUG_RAISE_INT(err);
  err = semctl(g_city_sems_cap, -1, IPC_RMID);
  DEBUG_RAISE_INT(err);
  err = msgctl(g_taxi_info_msq, IPC_RMID, NULL);
  DEBUG_RAISE_INT(err);
  err = msgctl(g_requests_msq, IPC_RMID, NULL);
  DEBUG_RAISE_INT(err);
  err = msgctl(g_taxi_spawn_msq, IPC_RMID, NULL);
  DEBUG_RAISE_INT(err);

  free(g_source_pids);
  free(g_sources_positions);
}

/* Signal handler del processo master */
void master_handler(int signum)
{
  int i, err, status;
  char selection;

  struct sembuf bufor_sem;

  switch (signum)
  {
  case SIGINT:                      /* User paused from terminal */
    send_signal_to_taxigen(SIGINT); /* taxigen must stop by itself other sub processes */
    send_signal_to_changedetector(SIGSTOP);
    send_signal_to_mastertimer(SIGSTOP);
    send_signal_to_sources(SIGSTOP);

    sleep_for(1, 0);
    fflush(stdout);
    fflush(stderr);
    printf("Press q to quit or any key to continue.\n");
    scanf("%c", &selection);
    if (selection != 'q')
    {
      /* continue */
      send_signal_to_taxigen(SIGCONT);
      send_signal_to_changedetector(SIGCONT);
      send_signal_to_mastertimer(SIGCONT);
      send_signal_to_sources(SIGCONT);
    }
    else
    {
      send_signal_to_taxigen(SIGTERM);
      send_signal_to_changedetector(SIGTERM);
      send_signal_to_mastertimer(SIGTERM);
      send_signal_to_sources(SIGTERM);

      clear_memory();
      exit(EXIT_FAILURE);
    }
    break;
  case SIGTERM: /* Interrupts the simulation - politely ask a program to terminate - can be blocked, handled, and ignored */
    send_signal_to_taxigen(SIGTERM);
    send_signal_to_changedetector(SIGTERM);
    send_signal_to_mastertimer(SIGTERM);
    send_signal_to_sources(SIGTERM);

    clear_memory();
    exit(EXIT_FAILURE);
    break;
  case SIGUSR1: /* Request new ride in every source */
    send_signal_to_sources(SIGUSR1);
    break;
  case SIGUSR2: /* Interrupts the simulation - gracefully */
    send_signal_to_taxigen(SIGUSR2);
    send_signal_to_sources(SIGTERM);

    err = waitpid(g_taxigen_pid, &status, 0);
    DEBUG_RAISE_INT(err);

    send_signal_to_changedetector(SIGTERM);

    err = waitpid(g_changedetector_pid, &status, 0);
    DEBUG_RAISE_INT(err);
    
    clear_memory();
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}

void send_signal_to_taxigen(int signal)
{
  if (g_taxigen_pid != -1)
  {
    kill(g_taxigen_pid, signal);
  }
}

void send_signal_to_changedetector(int signal)
{
  if (g_changedetector_pid != -1)
  {
    kill(g_changedetector_pid, signal);
  }
}

void send_signal_to_mastertimer(int signal)
{
  if (g_mastertimer_pid != -1)
  {
    kill(g_mastertimer_pid, signal);
  }
}

void send_signal_to_sources(int signal)
{
  int i;
  for (i = 0; i < SO_SOURCES; i++)
  {
    if (g_source_pids[i] != -1)
    {
      kill(g_source_pids[i], signal);
    }
  }
}

/* Genera un punto di origine per source sulla città */
int generate_origin_point(int city_id, int city_sems_op)
{
  City city = shmat(city_id, NULL, 0);
  int pos = -1, done = FALSE, err;

  while (!done)
  {
    pos = rand_int(0, SO_WIDTH * SO_HEIGHT - 1);

    if (city[pos].type == CELL_NORMAL)
    {
      done = TRUE;
    }
  }

  /* Access the resource */
  err = sem_op(city_sems_op, pos, -1, 0);
  DEBUG_RAISE_INT(err);

  city[pos].type = CELL_SOURCE;

  /* Release the resource */
  err = sem_op(city_sems_op, pos, 1, 0);
  DEBUG_RAISE_INT(err);

  shmdt(city);
  return pos;
}

/* Crea un processo sorgente */
void create_source(int position)
{
  char str[5];
  sprintf(str, "%d", position);
  char *args[3] = {"source.o", str, NULL};
  int err;
  err = execve(args[0], args, environ);

  if (err == -1)
  {
    DEBUG;
    kill(getppid(), SIGTERM);
    exit(EXIT_FAILURE);
  }
}

/* Genera una lista dei punti intorno a p */
void generate_adjacent_list(Point p, int list[])
{
  Point tmp;
  int dx, dy;
  int i = 0;

  for (dx = -1; dx <= 1; dx++)
  {
    for (dy = -1; dy <= 1; dy++)
    {
      if (!(dx == 0 && dy == 0))
      {
        tmp.x = p.x + dx;
        tmp.y = p.y + dy;

        list[i] = point2index(tmp);
        i++;
      }
    }
  }
}

/* Verifica che p sia un punto valido per una buca */
int is_valid_hole_point(Point p, City city)
{
  int list[ADJACENT_CELLS_NUM];
  int i, is_valid = TRUE;
  int pos;

  generate_adjacent_list(p, list);

  /*TODO: use while(is_valid && i < 8) */
  for (i = 0; i < 8; i++)
  {
    pos = list[i]; /* per chiarezza */

    if (pos >= 0 && pos < SO_WIDTH * SO_HEIGHT)
    {
      is_valid = is_valid && city[pos].type != CELL_HOLE;
    }
  }

  return is_valid;
}

/* Posiziona una buca in pos */
void place_hole(int pos, City city)
{
  city[pos].capacity = -1;
  city[pos].cross_time = -1;
  city[pos].type = CELL_HOLE;
  city[pos].crossing_num = -1;
  city[pos].act_capacity = -1;
}

/* Inizializza una struct taxi */
void init_taxi(TaxiStats *taxi)
{
  taxi->crossed_cells = -1;
  taxi->max_travel_time = -1;
  taxi->requests = -1;
}