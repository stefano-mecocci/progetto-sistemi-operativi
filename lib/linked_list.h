#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include <sys/types.h>
#include "data_structures.h"

/* Add a node to the list */
extern List list_add(List p, pid_t pid);

/* Free all the list nodes */
extern void list_free(List p);

/* Increase the crossed_cells field of a node elem by pid */
extern void list_increase_taxi_crossed_cells(List p, pid_t pid);

/* Increase the requests field of a node elem by pid */
extern void list_increase_taxi_requests(List p, pid_t pid);

/*
Print the elem of each node of the list
for debug purposes
*/
extern void list_print(List p);

#endif