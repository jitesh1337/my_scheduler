SRCS =  mysched.c mytest.c
INC = futex.h  futex_inline.h  myatomic.h  mythread.h  myqueue.h mymutex.h mycond.h mythread_priv.h
OBJS = $(SRCS:.c=.o)
LIB = mythread.a

CFLAGS = -Wall -Werror -I.
LDFLAGS = -L.
EXTRA_CFLAGS = -g

CC = gcc

a5:  all
all: lib mytest
test: mytest

lib: $(OBJS) $(INC)

%.o: %.c $(INC)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

mytest: mysched.o mytest.o $(LIB) $(INC)
	$(CC) -o mytest $(CFLAGS) $(EXTRA_CFLAGS) mysched.o mytest.o $(LIB)

clean:
	rm -f $(OBJS) mytest *~
tags:
	find . -name "*.[cChH]" | xargs ctags
	find . -name "*.[cChH]" | etags -





