#Single Author info:
# 	(All of us contributed equal share)
#	jhshah  Jitesh H  Shah
#	sskanitk Salil S Kanitkar
#	msinha	Mukul Sinha
#Group info:
#      jhshah     Jitesh H Shah
#      sskanitk  Salil S Kanitkar
#	msinha	Mukul Sinha


SRCS =  mysched.c mytest.c
INC = futex.h  futex_inline.h  myatomic.h  mythread.h  myqueue.h mymutex.h mycond.h mythread_priv.h
OBJS = $(SRCS:.c=.o)
LIB = mythread-new.a

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

mytest: $(OBJS) $(LIB) $(INC)
	$(CC) -o mytest $(CFLAGS) $(EXTRA_CFLAGS) $(OBJS) $(LIB)

mytest-2: mytest-2.o mysched.o $(LIB) $(INC)
	$(CC) -o mytest-2 $(CFLAGS) $(EXTRA_CFLAGS) mytest-2.o mysched.o $(LIB)

clean:
	rm -f $(OBJS) mytest mytest-2 *~
tags:
	find . -name "*.[cChH]" | xargs ctags
	find . -name "*.[cChH]" | etags -





