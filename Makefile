COMPILE      = gcc -g -Wall -Werror
COMPILE_OUT  = $(COMPILE) $^ -o$@
COMPILE_O    = $(COMPILE) -c $<

# shortcuts
all: server client
	@# cascade

server: server.out
	@# cascade

client: client.out
	@# cascade

# executables
server.out: server.o
	$(COMPILE_OUT)

client.out: client.o
	$(COMPILE_OUT)

# objects
server.o: server.c
	$(COMPILE_O)

client.o: client.c
	$(COMPILE_O)

# cleaning
clean-build: clean all
	@# cascade

clean:
	-rm *.o *.gch *.out

