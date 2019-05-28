PROG = chess
CC = gcc
CFLAGS =
LDFLAGS =
OBJS = main.o

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

main.o :
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f core $(PROG) $(OBJS)
