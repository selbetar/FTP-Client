all: ftp

#The following lines contain the generic build options
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration

#List all the .o files here that need to be linked 
OBJS=ftp.o network.o


network.o: network.c network.h common.h

ftp.o: ftp.c ftp.h common.h network.h


ftp: $(OBJS) 
	$(CC) -o ftp $(OBJS) $(CLIBS)

clean:
	rm -f *.o
	rm -f ftp
