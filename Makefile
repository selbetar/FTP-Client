all: ftp

#The following lines contain the generic build options
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration

OBJS=ftp.o network.o printRoutines.o

printRoutines.o : printRoutines.c printRoutines.h
network.o: network.c network.h common.h

ftp.o: ftp.c ftp.h common.h network.h printRoutines.h


ftp: $(OBJS) 
	$(CC) -o ftp $(OBJS) $(CLIBS)

clean:
	rm -f *.o
	rm -f ftp
