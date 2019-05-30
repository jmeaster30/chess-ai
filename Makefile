PROG = chess
CC = gcc
CFLAGS =
LDFLAGS =
OBJS = main.o sanparser.o

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

main.o :
	$(CC) $(CFLAGS) -c main.c

sanparser.o : sanparser.h
	$(CC) $(CFLAGS) -c sanparser.c

clean:
	rm -f core $(PROG) $(OBJS)
