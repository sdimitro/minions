#!/bin/sh

compile_all() {
	cd mfapi ; make; cd ..
	cd rio   ; make; cd ..
	cd ss    ; make; cd ..
	cd sbuf  ; make; cd ..

	cd dht
	cp ../mfapi/mfapi.o .
	cp ../rio/rio.o .
	cp ../ss/ss.o .
	cp ../sbuf/sbuf.o .
	make
	cd ..

	cd peer
	cp ../mfapi/mfapi.o .
	cp ../rio/rio.o .
	cp ../ss/ss.o .
	cp ../sbuf/sbuf.o .
	cp ../dht/*.o .
	make
	cd ..

}

clean_all() {
	cd mfapi ; make clean; cd ..
	cd rio   ; make clean; cd ..
	cd ss    ; make clean; cd ..
	cd sbuf  ; make clean; cd ..
	cd dht   ; make clean; cd ..
	cd peer  ; make clean; cd ..
}

if   [ "$1" = "compile" ]; then
	compile_all
elif [ "$1" = "clean"   ]; then
	clean_all
else
	echo "usage: builder.sh [compile|clean]"
fi
