CC=gcc
CFLAGS=-I.
DEPS = queue.h
OBJ = testafila.o queue.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

queue: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o
