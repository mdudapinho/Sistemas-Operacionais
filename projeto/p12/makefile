CC=gcc
CFLAGS=-I.
DEPS = queue.h datatypes.h pingpong.h
OBJ = pingpong.o queue.o pingpong-barrier.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pingpong: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o
