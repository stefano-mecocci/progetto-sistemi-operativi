#include <stdio.h>
#include "data_structures.h"
#include <stdlib.h>
#include "linked_list.h"
#include "utils.h"
#include <errno.h>

List list_add(List p, pid_t pid)
{
  List new_list;
  new_list = malloc(sizeof(Node));

  /* Inizializzo l'elemento del nodo */
  new_list->taxi_stats.pid = pid;
  new_list->taxi_stats.crossed_cells = 0;
  new_list->taxi_stats.longest_travel_time = 0;
  new_list->taxi_stats.requests = 0;

  /* Imposto il puntatore dopo */
  new_list->next = p;

  return new_list;
}

void list_free(List p)
{
  if (p == NULL)
  {
    return;
  }

  list_free(p->next);
  free(p);
}

void list_increase_taxi_crossed_cells(List p, pid_t pid)
{
  for (; p != NULL; p = p->next)
  {
    if (p->taxi_stats.pid == pid)
    {
      p->taxi_stats.crossed_cells++;
    }
  }
}

void list_increase_taxi_requests(List p, pid_t pid)
{
  for (; p != NULL; p = p->next)
  {
    if (p->taxi_stats.pid == pid)
    {
      p->taxi_stats.requests++;
    }
  }
}

void list_print(List p)
{
  TaxiStats tmp;

  for (; p != NULL; p = p->next)
  {
    tmp = p->taxi_stats;
    printf("[pid=%d, crossed_cells=%d, requests=%d]", tmp.pid, tmp.crossed_cells, tmp.requests);

    if (p->next != NULL)
    {
      printf(" -> ");
    }
  }

  printf(" -> NULL\n");
}