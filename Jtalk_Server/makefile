EXECUTABLES = jtalk jtalk_server



SOCKLIB = socketfun.o
LIB = libfdr.a

CC = gcc

all: $(EXECUTABLES)

clean:
	rm -f core *.o $(EXECUTABLES) a.out

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c


jtalk: jtalk.o
	$(CC) $(CFLAGS) -o jtalk jtalk.o -lpthread $(SOCKLIB) $(LIB)


jtalk_server:jtalk_server.o
	$(CC) $(CFLAGS) -o jtalk_server jtalk_server.o -lpthread $(SOCKLIB) $(LIB)
 

