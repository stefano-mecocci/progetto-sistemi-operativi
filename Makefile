CC = gcc
CFLAGS = -std=c89
OBJ = ./obj/

compile: create-dirs utils.o params.o
	@echo "[\033[0;32mINFO\033[0m] compiling master..."
	@$(CC) $(CFLAGS) -o $(OBJ)master.o $(OBJ)params.o master.c master_main.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxigen..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxigen.o $(OBJ)params.o taxigen.c taxigen_main.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxi..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxi.o $(OBJ)params.o taxi.c taxi_main.c $(OBJ)utils.o astar/astar.c astar/astar_heap.c sem_lib.c -lm
	@echo "[\033[0;32mINFO\033[0m] compiling change_detector..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxi_change_detector.o $(OBJ)params.o linked_list.c taxi_change_detector.c $(OBJ)utils.o sem_lib.c -lm
	@echo "[\033[0;32mINFO\033[0m] compiling master_timer..."
	@$(CC) $(CFLAGS) -o $(OBJ)master_timer.o $(OBJ)params.o master_timer.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling source..."
	@$(CC) $(CFLAGS) -o $(OBJ)source.o $(OBJ)params.o source_main.c source.c $(OBJ)utils.o -lm

create-dirs:
	@mkdir -p obj ipc_res out

utils.o:
	@echo "[\033[0;32mINFO\033[0m] compiling utils..."
	@$(CC) $(CFLAGS) -c utils.c  -o $(OBJ)utils.o

params.o:
	@echo "[\033[0;32mINFO\033[0m] compiling params..."
	@$(CC) $(CFLAGS) -c params.c  -o $(OBJ)params.o

clean:
	@rm -rf $(OBJ)*.o ./ipc_res/*.txt

run-one: compile
	./one.sh

run-dense: compile
	./dense.sh

run-large: compile
	./large.sh	
