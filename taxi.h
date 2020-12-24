#ifndef _TAXI_H
#define _TAXI_H

int get_sync_sem();

void sem_remove(int sem_arr, int sem, int op);

void set_handler();

#endif