CC=gcc
CFLAGS=-I.
DEPS = queue.h datatypes.h pingpong.h
OBJ = pingpong.o queue.o pingpong-mqueue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -lm

pingpong: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lm

clean:
	rm -rf *.o
