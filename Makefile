CC=g++
CFLAGS=-Wall -c -g  
INCLUDE=-I/home/scf-22/csci551b/openssl/include
LIB=-L/home/scf-22/csci551b/openssl/lib
OPT=-lcrypto -lsocket -lnsl -lresolv 
LFLAGS= $(OPT) 

all:server client common.o
server:server.o common.o
	$(CC) -o server server.o common.o $(LFLAGS)
client:client.o common.o
	$(CC) -o client client.o common.o $(LFLAGS)
server.o:server.cc server.h 
	$(CC) $(CFLAGS) server.cc $(INCLUDE)
client.o:client.cc client.h
	$(CC) $(CFLAGS) client.cc $(INCLUDE)
common.o:common.cc 
	$(CC) $(CFLAGS) common.cc

clean:
	rm -rf server client *.o
