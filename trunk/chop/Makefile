SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS = -Dchop_MYSQL
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -L../StormLib/stormlib/ -lbncsutil -lpthread -ldl -lz -lStorm -lmysqlclient_r -lboost_date_time -lboost_thread -lboost_system -lpython2.7 -lboost_python -lgmp
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../bncsutil/src/ -I../StormLib/ -I/usr/include/python2.7

ifeq ($(SYSTEM),Darwin)
CFLAGS += -I../mysql/include/
endif

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o bnlsclient.o bnlsprotocol.o config.o command.o commandpacket.o crc32.o chop.o db.o dbmysql.o language.o socket.o user.o util.o
PROGS = ./chop++

all: $(OBJS) $(COBJS) $(PROGS)

./chop++: $(OBJS) $(COBJS)
	$(C++) -o ./chop++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./chop++: $(OBJS) $(COBJS)

all: $(PROGS)

bncsutilinterface.o: chop.h util.h bncsutilinterface.h
bnet.o: chop.h util.h config.h language.h socket.h commandpacket.h db.h bncsutilinterface.h bnlsclient.h bnetprotocol.h bnet.h
bnetprotocol.o: chop.h util.h bnetprotocol.h
bnlsclient.o: chop.h util.h socket.h commandpacket.h bnlsprotocol.h bnlsclient.h
bnlsprotocol.o: chop.h util.h bnlsprotocol.h
command.o: chop.h util.h config.h language.h socket.h commandpacket.h db.h bncsutilinterface.h bnlsclient.h bnetprotocol.h bnet.h user.h
commandpacket.o: chop.h commandpacket.h
config.o: chop.h config.h
crc32.o: chop.h crc32.h
chop.o: chop.h util.h crc32.h config.h language.h socket.h db.h dbmysql.h bnet.h
db.o: chop.h util.h config.h db.h
dbmysql.o: chop.h util.h config.h db.h dbmysql.h
language.o: chop.h config.h language.h
socket.o: chop.h util.h socket.h
user.o: chop.h user.h
util.o: chop.h util.h
