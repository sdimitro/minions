CFLAGS= -g -O2 -Wpedantic -std=c11 -Wall -Wextra
ISS= -I ../ss
ISBUF= -I ../sbuf
IMFAPI= -I ../mfapi

all: dht dht_client

dht_client: dht_conf dht_api
	gcc $(CFLAGS) $(ISS) $(IMFAPI) -o dhtc dht_client.c msg.o ss.o mfapi.o dht_conf.o ht.o dht_api.o

dht_api: dht_conf
	gcc $(CFLAGS) $(ISS) $(IMFAPI) -c dht_api.c

dht_conf:
	gcc $(CFLAGS) $(IMFAPI) -c dht_conf.c

dht: msg ht
	gcc $(CFLAGS) $(ISS) $(ISBUF) -o dht dht.c msg.o ht.o sbuf.o ss.o -lpthread

msg:
	gcc $(CFLAGS) -c msg.c

ht:
	gcc $(CFLAGS) -c ht.c

clean:
	rm -rf *.o
