CC=gcc
CFLAGS=-I.
DEPS = queue.h datatypes.h pingpong.h
OBJ = pingpong.o queue.o pingpong-join.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pingpong: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o
