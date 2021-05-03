COMPILE      = gcc -g -Wall -Werror
COMPILE_OUT  = $(COMPILE) $^ -o$@
COMPILE_O    = $(COMPILE) -c $<

server: server.out
	@# cascade

server.out: server.o
	$(COMPILE_OUT)

server.o: server.c
	$(COMPILE_O)

clean:
	-rm *.o *.gch *.out

