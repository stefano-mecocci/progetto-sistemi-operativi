CC = gcc
CFLAGS = -std=c89 -pedantic

MASTER = master.c main.c
MASTER_EXE = main

compile: clean
	@$(CC) $(CFLAGS) -o $(MASTER_EXE) $(MASTER)

clean:
	@rm -f $(MASTER_EXE)