CFLAGS= -g -O2 -Wpedantic -std=c11 -Wall -Wextra
IRIO= -I ../rio
IDHT= -I ../dht
ISS= -I ../ss
ISBUF= -I ../sbuf
IMFAPI= -I ../mfapi
IALL= $(IRIO) $(ISS) $(ISBUF) $(IMFAPI) $(IDHT)

compile: peer_func
	gcc $(CFLAGS) $(IALL) -o peer -g peerd.c peer.c ss.o mfapi.o dht_api.o dht_conf.o ht.o peer_func.o msg.o rio.o sbuf.o -lpthread

peer_func:
	gcc $(CFLAGS) $(IMFAPI) $(IDHT) -c peer_func.c

peerd:
	gcc $(CFLAGS) $(IALL) -c peerd.c -lpthread

clean:
	rm -f a.out peer *.o
