CC = gcc
CFLAGS = -std=c89 -pedantic

compile: clean
	@$(CC) $(CFLAGS) -o main master.c main.c
	@$(CC) $(CFLAGS) -o taxi taxi.c taxi_main.c

clean:
	@rm -f main taxi