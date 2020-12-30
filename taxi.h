#ifndef _TAXI_H
#define _TAXI_H

/* Ottieni ID del semaforo sync taxi-master */
int get_sync_sem();

/* Ottieni ID della coda di messaggi taxi-requests */
int get_requests_msq();

/* Rimuovi value dal semaforo sem in sem_arr */
void sem_decrease(int sem_arr, int sem, int value);

/* Imposta il signal handler di taxi */
void set_handler();

void take_request(int requests_msq);

#endif