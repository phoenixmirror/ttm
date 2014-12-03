
CC	=	gcc -g -O2 -fPIC 

OBJS	=	$(C_SRCS:.c=.o)

all	:	clean ttm $(OBJS)

C_SRCS	=	common.c ttm.c main.c worker.c


CFLAGS	=	-I.                             \
		-I$(HOME)/include               \
		-I$(HOME)/include/ib2api        \
		-I$(HOME)/include/ib2core       \
		-I$(HOME)/include/ib2util       \
		-I$(HOME)/include/ib2ssd        \

LFLAGS	=	-L.                             \
		-L$(HOME)/lib                   \
		-lib2api                        \
		-lib2core                       \
		-lib2util                       \
		-lib2fmd                        \
		-lib2ssd                        \
		-lhzb_log                       \
		-ldbutil                        \
		-lIButil                        \
		-ldbutil8                       \
		-lxml2				\


ttm	:	$(OBJS)	
	@echo 'Building target: $@'
	$(CC) -o $@ $(LFLAGS) $(OBJS)

.c.o	:
	@echo 'Building target: $@'
	$(CC) $(CFLAGS) -c $<

clean	:
	rm -f *.o
	rm -f ttm

