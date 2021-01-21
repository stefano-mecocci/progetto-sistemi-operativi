CC = gcc
CFLAGS = -std=c89 -pedantic

ID_FILES = city_id 
ID_FILES += city_sems_op city_sems_cap sync_sems
ID_FILES += taxi_info_msq taxi_spawn_msq requests_msq

compile: clean
	@$(CC) $(CFLAGS) -o master master.c master_main.c
	@$(CC) $(CFLAGS) -o taxigen taxigen.c taxigen_main.c
	@$(CC) $(CFLAGS) -o taxi taxi.c taxi_main.c
	@$(CC) $(CFLAGS) -o taxi_timer taxi_timer.c
	@$(CC) $(CFLAGS) -o master_timer master_timer.c
	@$(CC) $(CFLAGS) -o source source.c source_main.c

clean:
	@rm -f master taxigen taxi source master_timer taxi_timer
	@rm -f $(ID_FILES)

test:
	@$(CC) $(CFLAGS) test.c && ./a.out
	@rm -rf a.out