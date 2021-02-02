CC = gcc
CFLAGS = -std=c89
OBJ = ./obj/

compile: create-dirs utils.o params.o
	@echo "[\033[0;32mINFO\033[0m] compiling master..."
	@$(CC) $(CFLAGS) -o $(OBJ)master.o master.c \
		$(OBJ)params.o lib/master_lib.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxigen..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxigen.o taxigen.c \
		$(OBJ)params.o lib/taxigen_lib.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxi..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxi.o taxi.c \
		$(OBJ)params.o lib/taxi_lib.c $(OBJ)utils.o \
		lib/astar/pathfinder.c -lm
	@echo "[\033[0;32mINFO\033[0m] compiling change_detector..."
	@$(CC) $(CFLAGS) -o $(OBJ)taxi_change_detector.o taxi_change_detector.c \
		$(OBJ)params.o lib/linked_list.c $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling master_timer..."
	@$(CC) $(CFLAGS) -o $(OBJ)master_timer.o master_timer.c \
		$(OBJ)params.o $(OBJ)utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling source..."
	@$(CC) $(CFLAGS) -o $(OBJ)source.o source.c \
		$(OBJ)params.o lib/source_lib.c $(OBJ)utils.o -lm

create-dirs:
	@mkdir -p obj ipc_res out

utils.o:
	@echo "[\033[0;32mINFO\033[0m] compiling utils..."
	@$(CC) $(CFLAGS) -c lib/utils.c -o $(OBJ)utils.o

params.o:
	@echo "[\033[0;32mINFO\033[0m] compiling params..."
	@$(CC) $(CFLAGS) -c lib/params.c  -o $(OBJ)params.o

clean:
	@rm -rf $(OBJ)*.o ./ipc_res/* ./out/*

run-one: compile
	./one.sh

run-dense: compile
	./dense.sh

run-large: compile
	./large.sh	
