# The taxicab game
## Progetto per il corso di Sistemi Operativi UniTO 2020/2021
Simulation of concurrent processes execution represented by taxi, serving customers around the city

# Oggetti IPC usati

- 3 code: `taxi_info`, `taxi_spawn` e `requests`
- 1 coda per ogni source: `origin_msq`
- 3 array di semafori: `sync_sems`, `city_sems_op`, `city_sems_cap`
- 1 memoria condivisa: `city`

# Strutture usate
Una richiesta taxi è una struct così definita:
```c
typedef struct request {
  long mtype;
  int mtext[2];
} Request;
```
dove `mtext` contiene due interi che indicano rispettivamente origine e destinazione.

Un taxi è definito come struct:
```c
typedef struct taxi {
  int crossed_cells;
  int max_travel_time;
  int requests;
} Taxi;
```
dove:
- `crossed_cells` indica il numero di celle percorse
- `max_travel_time` indica il tempo del viaggio più lungo che ha fatto
- `requests` indica il numero di richieste che ha preso in carico

Una creazione di taxi è definita come messaggio (struct):
```c
typedef struct spawn {
  long mtype;
  int mtext[2];
} Spawn;
```
dove `mtext` contiene due interi ma al momento ne uso solo 1 che contiene, se serve, la posizione del taxi vecchio da rimuovere

Una cella è definita come struct:
```c
typedef struct cell {
  int type;
  int capacity;
  int act_capacity;
  int cross_time;
  int crossing_num;
} Cell;
```
dove:
- `type` indica il tipo (normal, source o hole)
- `act_capacity` indica la capacità attuale
- `capacity` indica la capacità della cella di ospitare taxi
- `cross_time` indica il tempo di attraversamento in nanosecondi
- `crossing_num` indica quante volte è stata attraversata

# Funzionalità
Il **master** per creare i taxi invia `SO_TAXI` messaggi spawn sulla coda `taxi_spawn`

Il processo **master** crea le risorse IPC che servono e ne salva l'ID di ognuna in un file apposito. Esempio:
- id = 123
- nome risorsa = miacoda
- nella cartella ci sarà un file con nome miacoda e contenuto 123

Il processo **taxigen** attende infinitamente messaggi spawn sulla coda `taxi_spawn`. Se ne arriva uno:
- rimuove il vecchio taxi sulla memoria condivisa (aumenta la capacità della cella) se necessario
- imposta il nuovo taxi (diminuisce la capacità della cella)
- crea l'effettivo e nuovo processo taxi passando come argomenti
  - il pid del **master**
  - un valore booleano che indica se il taxi è nuovo (creato dal precedente taxi)
  
Inoltre **taxigen** salva in un var. globale tutti i pid dei processi taxi attualmente in vita, per ucciderli poi tramite kill().

Ogni processo **source** ha una coda associata `origin_msq` per ricevere il punto d'origine come messaggio. Il quale verrà salvato poi come var. globale. Fatto ciò ogni 3 secondi crea ed invia sulla coda `requests` una richiesta taxi.

L'array di semafori `sync_sems` viene inizializzato così:
- all'indice 0 con SO_TAXI (sincronizzazione creazione taxi)
- all'indice 1 con SO_SOURCES (sincronizzazione creazione sources)
- all'indice 2 con 0 (numero di processi taxi attualmente vivi)

L'array di semafori `city_sems_op` indica se si può scrivere su una data cella:
- se il semaforo `i` vale 1 si può scrivere, se 0 no

Inoltre sia il master che ogni processo taxi forkano un processo che fa da timer:
- ossia per il master dopo `SO_DURATION` secondi arriva un segnale `SIGUSR2`
- per ogni taxi dopo `SO_TIMEOUT` secondi arriva `SIGUSR1`

Se un taxi muore per timer:
- invia le sue informazioni sulla coda `taxi_info` (letta poi da master)
- invia una richiesta di spawn a taxigen sulla coda `taxi_spawn`
- diminuisce di 1 il semaforo per i taxi vivi (`sync_sems[2]`)

# Segnali

Se invio `SIGTERM` al master questo:
- invia `SIGTERM` a tutti processi source
- invia `SIGTERM` a taxigen (che a sua volta lo invia a tutti i taxi)
- rimuove risorse IPC
- termina
  
`SIGTERM` è per debug praticamente

Se invio `SIGUSR2` al master questo:
- fa lo stesso discorso di `SIGTERM` ma con `SIGUSR2`
- aspetta che `sync_sems[2]` valga 0 (zero processi taxi vivi)
- aggiorna e stampa le statistiche (legge coda `taxi_info`)
- rimuove risorse IPC
- esce

Se invio un segnale `SIGUSR1` al processo **master** questo lo invia ad ogni source
Quando ad una source arriva `SIGUSR1` questa crea una richiesta taxi e la invia

