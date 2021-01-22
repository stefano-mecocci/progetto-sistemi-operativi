CC = gcc
CFLAGS = -std=c89 -pedantic

compile: clean
	@$(CC) $(CFLAGS) -o master.o master.c master_main.c utils.c
	@$(CC) $(CFLAGS) -o taxigen.o taxigen.c taxigen_main.c utils.c
	@$(CC) $(CFLAGS) -o taxi.o taxi.c taxi_main.c utils.c
	@$(CC) $(CFLAGS) -o taxi_timer.o taxi_timer.c utils.c
	@$(CC) $(CFLAGS) -o taxi_change_detector.o taxi_change_detector.c utils.c
	@$(CC) $(CFLAGS) -o master_timer.o master_timer.c utils.c
	@$(CC) $(CFLAGS) -o source.o source_main.c source.c utils.c
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c

clean:
	@rm -rf *.o

run-dense: compile
	./dense.sh

run-large: compile
	./large.sh	

test:
	@$(CC) $(CFLAGS) test.c && ./a.out
	@rm -rf a.out

path_finder.o:
	@$(CC) $(CFLAGS) -o path_finder.o path_finder.c astar/astar.c astar/astar_heap.c

test-astar: path_finder.o
	./path_finder.o
