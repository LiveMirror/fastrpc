CC= g++
RANLIB= ranlib


DEBUG = -g
#DEBUG = -O3


CFLAGS= -Wall -DUNIX $(DEBUG) -Wno-deprecated -D_REENTRANT
CFLAGS+=-I../ependingpool/output/include/ -I../ullib/include
CFLAGS+=-I../http_codec/api -I./

OBJS = CSvr.o CCliNetD.o CBackNetD.o CCliNetD.o CMainD.o CNetfun.o CRWCache.o CTcpSocket.o
OBJS +=  CLog.o Interaction.o CThreadSync.o CCallbackObj.o Statistic.o CTimerContainer.o
LIB = libasvr.a

all:$(LIB)
$(LIB):
	rm -rf $@
	rm -rf *.o
	$(CC) $(CFLAGS) -c CTimerContainer.cpp
	$(CC) $(CFLAGS) -c Statistic.cpp
	$(CC) $(CFLAGS) -c CCallbackObj.cpp
	$(CC) $(CFLAGS) -c CThreadSync.cpp
	$(CC) $(CFLAGS) -c Interaction.cpp
	$(CC) $(CFLAGS) -c CLog.cpp
	$(CC) $(CFLAGS) -c CTcpSocket.cpp
	$(CC) $(CFLAGS) -c CRWCache.cpp
	$(CC) $(CFLAGS) -c CNetfun.cpp
	$(CC) $(CFLAGS) -c CMainD.cpp
	$(CC) $(CFLAGS) -c CCliNetD.cpp
	$(CC) $(CFLAGS) -c CBackNetD.cpp
	$(CC) $(CFLAGS) -c CCliNetD.cpp
	$(CC) $(CFLAGS) -c CSvr.cpp
	ar q $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -rf *.o
	rm -rf core
	rm -f libasvr.a
