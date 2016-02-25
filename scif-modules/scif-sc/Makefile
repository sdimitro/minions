compile:
	icc -mmic -c -fpic -g gemtc_api.c
	icc -mmic -c -fpic -g master_proc.c
	icc -mmic -c -fpic -g mtc_queue.c
	icc -mmic -c -fpic -g mtc_apps/mtc_apps.c

link: compile
	icc -mmic -shared -o libmtcq.so.1 gemtc_api.o master_proc.o mtc_queue.o mtc_apps.o -lpthread

clean:
	rm -f libmtcq.so.1 gemtc_api.o master_proc.o mtc_queue.o mtc_apps.o

LDLIBPATH=/software/intel/composer_xe_2015.1.133/compiler/lib/intel64/
SLDLIBPATH=/software/intel/composer_xe_2015.1.133/compiler/lib/mic:/home/serapheimd/repos/scif-modules/scif-sc/
compile-scif: link
	icc -mmic -Wall -Wextra -g scif/server.c libmtcq.so.1 -I . -o scif/server -lpthread -lscif
	icc -m64 -Wall -Wextra -g scif/scif_client_helper.c scif/sleep_client.c -o scif/sleep_client -lscif
	icc -m64 -Wall -Wextra -g scif/scif_client_helper.c scif/shutdown_client.c -o scif/shutdown_client -lscif
	icc -m64 -Wall -Wextra -g scif/scif_client_helper.c scif/dequeue_client.c -o scif/dequeue_client -lscif
	icc -m64 -Wall -Wextra -g scif/scif_client_helper.c scif/matrix_client.c -o scif/matrix_client -lscif

show-dep: compile-scif
	SINK_LD_LIBRARY_PATH=$(SLDLIBPATH) micnativeloadex scif/server -l

test-drive-server:
	SINK_LD_LIBRARY_PATH=$(SLDLIBPATH) micnativeloadex scif/server -a "2 10"
