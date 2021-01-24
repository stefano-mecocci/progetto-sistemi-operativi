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

#define ADJACENT_CELLS_NUM 8

/* OGGETTI IPC */
int g_city_id;        /* città */
int g_sync_sems;      /* semafori di sync */
int g_city_sems_op;   /* semafori per operare su celle */
int g_city_sems_cap;  /* semafori per controllare capacità */
int g_requests_msq;   /* Coda richieste */
int g_taxi_info_msq;  /* Coda informazioni statistiche */
int g_taxi_spawn_msq; /* Coda spawns */

int g_taxi_list_id;  /* id taxi_list shmem */
int g_taxi_list_sem; /* semaforo per leggere/scrivere nella taxi_list shmem */

/* ENTITÀ */
pid_t *g_source_pids;
pid_t g_taxigen_pid;
pid_t g_mastertimer_pid;
pid_t g_changedetector_pid;
int *g_sources_positions;

/* STATISTICHE */
int g_travels;           /* viaggi totali (successo, abortiti ecc.) */
int *g_top_cells;        /* posizione celle più attraversate */
Taxi g_most_street;      /* Taxi che ha percorso più celle */
Taxi g_most_long_travel; /* Taxi che ha fatto il viaggio più lungo */
Taxi g_most_requests;    /* Taxi che ha raccolto più richieste */

void init_taxi(Taxi *taxi);
void master_handler(int signum);
void clear_memory();
void generate_adjacent_list(Point p, int list[]);
int is_valid_hole_point(Point p, City city);
void place_hole(int pos, City city);
void create_source();
int generate_origin_point(int, int);
void send_source_origin(int origin_msq, int origin);
void add_taxi_info_msq_end();
void update_stats();
void update_taxi_stats(int taxi_msg[]);
void copy_taxi_data(int taxi_msg[], Taxi *taxi);
void print_stats();

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

int create_taxi_availability_list()
{
  int size = sizeof(TaxiStatus) * SO_TAXI;
  int id = shmget(IPC_PRIVATE, size, 0660 | IPC_CREAT);
  g_taxi_list_id = id;

  DEBUG_RAISE_INT(id);

  return id;
}

void init_taxi_availability_list()
{
  TaxiStatus *taxi = shmat(g_taxi_list_id, NULL, 0);
  int i;

  for (i = 0; i < SO_TAXI; i++)
  {
    taxi[i].pid = -1;
    taxi[i].available = FALSE;
    taxi[i].position = -1;
  }

  shmdt(taxi);
}

int create_taxi_availability_list_sem()
{
  int nsems = 1;
  int id = semget(IPC_PRIVATE, nsems, 0660 | IPC_CREAT);
  g_taxi_list_sem = id;

  DEBUG_RAISE_INT(id);

  return id;
}

void init_taxi_availability_list_sem()
{
  semctl(g_taxi_list_sem, 0, SETVAL, 1);
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

  g_travels = 0;
  g_top_cells = malloc(sizeof(int) * SO_TOP_CELLS);
  init_taxi(&g_most_street);
  init_taxi(&g_most_long_travel);
  init_taxi(&g_most_requests);

  DEBUG_RAISE_ADDR(g_source_pids);
  DEBUG_RAISE_ADDR(g_sources_positions);
  DEBUG_RAISE_ADDR(g_top_cells);

  for (i = 0; i < SO_SOURCES; i++)
  {
    g_source_pids[i] = -1;
    g_sources_positions[i] = -1;
  }

  for (i = 0; i < SO_TOP_CELLS; i++)
  {
    g_top_cells[i] = -1;
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
  err += semctl(sync_sems, SEM_ALIVES_TAXI, SETVAL, 0);

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
    semctl(city_sems_cap, i, SETVAL, city[i].capacity);
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
  Spawn req;

  for (i = 0; i < SO_TAXI; i++)
  {
    req.mtype = SPAWN;
    req.mtext[0] = -1;
    req.mtext[1] = -1;

    err = msgsnd(taxi_spawn_msq, &req, sizeof req.mtext, 0);
    DEBUG_RAISE_INT(err);
  }
}

void set_sources(int city_id, int city_sems_op)
{
  int i;

  for (i = 0; i < SO_SOURCES; i++)
  {
    g_sources_positions[i] = generate_origin_point(city_id, city_sems_op);
  }
}

void create_sources()
{
  pid_t pid;
  int i;

  for (i = 0; i < SO_SOURCES; i++)
  {
    pid = fork();

    DEBUG_RAISE_INT(pid);

    if (pid == 0)
    {
      create_source();
    }
    else
    {
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
    else
    {
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
  }

  printf("\n\n");
  shmdt(city);
}

void send_sources_origins()
{
  int origin_msq, i;

  for (i = 0; i < SO_SOURCES; i++)
  {
    origin_msq = msgget(g_source_pids[i], 0660);
    send_source_origin(origin_msq, g_sources_positions[i]);
  }
}

/*
====================================
  FUNZIONI "PRIVATE"
====================================
*/

/* Pulisce la memoria dagli oggetti IPC usati e non solo */
void clear_memory()
{
  int err = 0;

  err += shmctl(g_city_id, IPC_RMID, NULL);
  err += semctl(g_sync_sems, -1, IPC_RMID);
  err += semctl(g_city_sems_op, -1, IPC_RMID);
  err += semctl(g_city_sems_cap, -1, IPC_RMID);
  err += msgctl(g_taxi_info_msq, IPC_RMID, NULL);
  err += msgctl(g_requests_msq, IPC_RMID, NULL);
  err += msgctl(g_taxi_spawn_msq, IPC_RMID, NULL);
  err += shmctl(g_taxi_list_id, IPC_RMID, NULL);
  err += semctl(g_taxi_list_sem, -1, IPC_RMID);

  free(g_top_cells);
  free(g_source_pids);

  if (err < 0)
  {
    DEBUG;
    exit(EXIT_FAILURE);
  }
}

/* Signal handler del processo master */
void master_handler(int signum)
{
  int i, err;
  char selection;

  switch (signum)
  {
  case SIGINT: /* User paused from terminal */
    send_signal_to_taxigen(SIGINT); /* taxigen must stop by itself other sub processes */
    send_signal_to_changedetector(SIGSTOP);
    send_signal_to_mastertimer(SIGSTOP);
    send_signal_to_sources(SIGSTOP);

    scanf("Press q to quit or any key to continue. %c\n", &selection);
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
    }
    break;
  case SIGTERM: /* Interrupts the simulation - brutally */    
    send_signal_to_taxigen(SIGTERM);
    send_signal_to_changedetector(SIGTERM);
    send_signal_to_mastertimer(SIGTERM);
    send_signal_to_sources(SIGTERM);  

    clear_memory();
    exit(EXIT_ERROR);
    break;
  case SIGUSR1: /* Request new ride in every source */
    send_signal_to_sources(SIGUSR1);
    break;
  case SIGUSR2: /* Interrupts the simulation - gracefully */
    send_signal_to_taxigen(SIGUSR2);
    send_signal_to_sources(SIGUSR2);  

    /* TOCHECK */
    err = sem_op(g_sync_sems, SEM_ALIVES_TAXI, 0, 0);
    DEBUG_RAISE_INT(err);

    update_stats();
    print_stats();

    clear_memory();
    exit(EXIT_TIMER);
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

/* Stampa statistiche di partita */
void print_stats()
{
  printf("\n------------------------------------\n");
  printf("Viaggi totali: %d\n", g_travels);
  printf("Taxi che:\n");
  printf("- ha fatto più strada: %d\n", g_most_street.crossed_cells);
  printf("- ha fatto il viaggio più lungo: %d\n",
         g_most_long_travel.max_travel_time);
  printf("- ha raccolto più richieste: %d\n", g_most_requests.requests);
  printf("------------------------------------\n\n");
}

/* Aggiorna le statistiche della partita */
void update_stats()
{
  TaxiInfo msg;
  int err;

  while (TRUE)
  {
    err = msgrcv(g_taxi_info_msq, &msg, sizeof msg.mtext, 0, IPC_NOWAIT);

    if (errno != ENOMSG)
    {
      DEBUG_RAISE_INT(err);

      g_travels += msg.mtext[2];
      update_taxi_stats(msg.mtext);
    }
    else
    {
      break;
    }
  }
}

/* Aggiorna le statistiche di un taxi */
void update_taxi_stats(int taxi_msg[])
{
  if (taxi_msg[0] > g_most_street.crossed_cells)
  {
    copy_taxi_data(taxi_msg, &g_most_street);
  }

  if (taxi_msg[1] > g_most_long_travel.max_travel_time)
  {
    copy_taxi_data(taxi_msg, &g_most_long_travel);
  }

  if (taxi_msg[2] > g_most_requests.requests)
  {
    copy_taxi_data(taxi_msg, &g_most_requests);
  }
}

/* Copia i dati di un taxi dal messaggio alla struct */
void copy_taxi_data(int taxi_msg[], Taxi *taxi)
{
  taxi->crossed_cells = taxi_msg[0];
  taxi->max_travel_time = taxi_msg[1];
  taxi->requests = taxi_msg[2];
}

/*
Aggiunge un messaggio di fine coda per consentire
di leggerla tutta
*/
void add_taxi_info_msq_end()
{
  TaxiInfo info;
  int err;

  info.mtype = 2;
  info.mtext[0] = 0;
  info.mtext[1] = 0;
  info.mtext[2] = 0;

  err = msgsnd(g_taxi_info_msq, &info, sizeof info.mtext, 0);
  DEBUG_RAISE_INT(err);
}

/* Invia l'origine alla sorgnete */
void send_source_origin(int origin_msq, int origin)
{
  Origin msg;
  int err;

  msg.mtype = 1;
  msg.mtext[0] = origin;
  err = msgsnd(origin_msq, &msg, sizeof msg.mtext, 0);

  DEBUG_RAISE_INT(err);
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
void create_source()
{
  char *args[2] = {"source.o", NULL};
  int err;
  err = execve(args[0], args, environ);

  if (err == -1)
  {
    DEBUG;
    kill(getppid(), SIGTERM);
    exit(EXIT_ERROR);
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
void init_taxi(Taxi *taxi)
{
  taxi->crossed_cells = -1;
  taxi->max_travel_time = -1;
  taxi->requests = -1;
}