# The taxicab game
## Progetto per il corso di Sistemi Operativi UniTO 2020/2021
Simulation of concurrent processes execution represented by taxi, serving customers around the city

## Struttura del progetto
Il progetto è organizzato nelle seguenti cartelle:
- `./` : contiene i file .c che hanno un main e descrivono il workflow di ogni processo;
- `./lib` : contiene tutti gli headers con le rispettive implementazioni delle estensioni dei processi nella root (librerie);
- `./obj` : cartella contenente i file oggetto risultanti dalla compilazione;
- `./ipc` : contiene i file di testo in cui vengono salvati gli id delle risorse ipc condivise;
- `./assets` : contiene immagini e diagrammi che descrivono l'architettura e il flow della soluzione;
- `./*.sh` : shell scripts che configurano le variabili di ambiente e avviano la simulazione nelle diverse modalità (dense, large, ecc);
