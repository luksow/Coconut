PROG=	libcoconut.a
SRCS=	coconut.c
OBJS=	${SRCS:.c=.o}

CC=	gcc
CFLAGS= -std=c11 -O3 -Wall -pedantic -c -g

.PHONY: all clean

all: prog

prog: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)
	ar rcs $(PROG) $(OBJS)

clean:
	@- rm -f $(PROG)
	@- rm -f $(OBJS)
