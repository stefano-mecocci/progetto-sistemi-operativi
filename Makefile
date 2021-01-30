CC = gcc
CFLAGS = -std=c89 -lm

compile: clean utils.o
	@echo "[\033[0;32mINFO\033[0m] compiling master..."
	@$(CC) $(CFLAGS) -o master.o params.c master.c master_main.c utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxigen..."
	@$(CC) $(CFLAGS) -o taxigen.o params.c taxigen.c taxigen_main.c utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling taxi..."
	@$(CC) $(CFLAGS) -o taxi.o params.c taxi.c taxi_main.c utils.o astar/astar.c astar/astar_heap.c sem_lib.c -lm
	@echo "[\033[0;32mINFO\033[0m] compiling change_detector..."
	@$(CC) $(CFLAGS) -o taxi_change_detector.o params.c linked_list.c taxi_change_detector.c utils.o sem_lib.c -lm
	@echo "[\033[0;32mINFO\033[0m] compiling master_timer..."
	@$(CC) $(CFLAGS) -o master_timer.o params.c master_timer.c utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling source..."
	@$(CC) $(CFLAGS) -o source.o params.c source_main.c source.c utils.o -lm
	@echo "[\033[0;32mINFO\033[0m] compiling path_finder..."
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c utils.c -lm

utils.o:
	@echo "[\033[0;32mINFO\033[0m] compiling utils..."
	@$(CC) $(CFLAGS) -c utils.c 

clean:
	@rm -rf *.o *.txt

run-one: compile
	./one.sh


run-dense: compile
	./dense.sh

run-large: compile
	./large.sh	

test:
	@$(CC) $(CFLAGS) test.c && ./a.out
	@rm -rf a.out

path_finder.o:
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c utils.c -lm

test-astar: 
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c utils.c -lm
	./path_finder.o

print-queue:
	@$(CC) $(CFLAGS) -o print_queue.o print_queue.c utils.c -lm
	./print_queue.o
