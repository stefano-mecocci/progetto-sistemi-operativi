CC = gcc
IDIR =./lib
ODIR=./obj
CFLAGS=-I$(IDIR) -std=c89 -pedantic

compile: create-dirs master taxigen taxi taxi_change_detector master_timer source

create-dirs:
	@mkdir -p obj ipc_res out

# master objects
master: $(ODIR)/master.o $(ODIR)/master_lib.o $(ODIR)/params.o $(ODIR)/utils.o
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm

$(ODIR)/master.o: master.c 
	@echo "[\033[0;32mINFO\033[0m] compiling master..."
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/master_lib.o: lib/master_lib.c lib/master_lib.h
	$(CC) -c -o $@ $< $(CFLAGS)

#taxigen objects
taxigen: $(ODIR)/taxigen.o $(ODIR)/taxigen_lib.o $(ODIR)/params.o $(ODIR)/utils.o 
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm

$(ODIR)/taxigen.o: taxigen.c 
	@echo "[\033[0;32mINFO\033[0m] compiling taxigen..."
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/taxigen_lib.o: lib/taxigen_lib.c lib/taxigen_lib.h
	$(CC) -c -o $@ $< $(CFLAGS)

#taxi objects
taxi: $(ODIR)/taxi.o $(ODIR)/taxi_lib.o $(ODIR)/pathfinder.o $(ODIR)/params.o $(ODIR)/utils.o 
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm

$(ODIR)/taxi.o: taxi.c 
	@echo "[\033[0;32mINFO\033[0m] compiling taxi..."
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/taxi_lib.o: lib/taxi_lib.c lib/taxi_lib.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/pathfinder.o: lib/astar/pathfinder.c lib/astar/pathfinder.h lib/astar/general.h
	$(CC) -c -o $@ $< $(CFLAGS)

#taxi_change_detector objects
taxi_change_detector: $(ODIR)/taxi_change_detector.o $(ODIR)/linked_list.o $(ODIR)/params.o $(ODIR)/utils.o 
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm

$(ODIR)/taxi_change_detector.o: taxi_change_detector.c 
	@echo "[\033[0;32mINFO\033[0m] compiling taxi_change_detector..."
	@$(CC) -c -o $@ $< $(CFLAGS)

#master_timer objects
master_timer: $(ODIR)/master_timer.o $(ODIR)/params.o $(ODIR)/utils.o 
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm
	
$(ODIR)/master_timer.o: master_timer.c
	@echo "[\033[0;32mINFO\033[0m] compiling master_timer..."
	@$(CC) -c -o $@ $< $(CFLAGS)

#source objects
source: $(ODIR)/source.o $(ODIR)/source_lib.o $(ODIR)/params.o $(ODIR)/utils.o 
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) -lm
	
$(ODIR)/source.o: source.c
	@echo "[\033[0;32mINFO\033[0m] compiling source..."
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/source_lib.o: lib/source_lib.c lib/source_lib.h
	$(CC) -c -o $@ $< $(CFLAGS)

#common objects
$(ODIR)/utils.o: lib/utils.c lib/utils.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/params.o: lib/params.c lib/params.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/linked_list.o: lib/linked_list.c lib/linked_list.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	@rm -rf $(ODIR)/* ./ipc_res/* ./out/*

run-one: compile
	./one.sh

run-dense: compile
	./dense.sh

run-large: compile
	./large.sh	
