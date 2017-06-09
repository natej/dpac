#
# dpac Makefile
#

CC = gcc

# debug
#CCOPT = -Wall -O0 -ggdb
# prod
CCOPT = -Wall -O2

#INCLS = -I../include

# Standard CFLAGS
CFLAGS = $(CCOPT) $(INCLS)

# Standard LDFLAGS
#LDFLAGS = -L/usr/lib

# Standard LIBS
#LIBS = -lbsd

#
# targets
#

dpac : dpac.h dpac.c alog.h alog.c ip.h ip.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ alog.c ip.c dpac.c $(LIBS)

clean :
	rm -f dpac *.o *~
