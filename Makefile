CC = gcc
CFLAGS = -std=c89 -lm

compile: clean utils.o
	@$(CC) $(CFLAGS) -o master.o master.c master_main.c utils.o -lm
	@$(CC) $(CFLAGS) -o taxigen.o taxigen.c taxigen_main.c utils.o -lm
	@$(CC) $(CFLAGS) -o taxi.o taxi.c taxi_main.c utils.o astar/astar.c astar/astar_heap.c sem_lib.c -lm
	@$(CC) $(CFLAGS) -o taxi_change_detector.o linked_list.c taxi_change_detector.c utils.o sem_lib.c -lm
	@$(CC) $(CFLAGS) -o master_timer.o master_timer.c utils.o -lm
	@$(CC) $(CFLAGS) -o source.o source_main.c source.c utils.o -lm
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c utils.c -lm
	@$(CC) $(CFLAGS) -o print_queue.o print_queue.c utils.o -lm

utils.o: 
	@$(CC) $(CFLAGS) -c utils.c 

clean:
	@rm -rf *.o changes.txt

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
