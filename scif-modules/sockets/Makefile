SO=		libmtcq.so.1
CC=		icc
CFLAGS_DEBUG=	-m64 -fPIC -std=c11 -W -Wall -Wextra -g
CFLAGS_NDEBUG=	-m64 -fPIC -W -Wall -Wextra
CFLAGS=		$(CFLAGS_DEBUG)
LIBS=		-lpthread -lc
# On Illumos, DTrace won't work correctly if -h isn't set
LDFLAGS_ILLUM=	-shared -h $(SO)
LDFLAGS_LINUX=	-shared
LDFLAGS=	$(LDFLAGS_LINUX)

# Really?
fuckit:
	echo $(CFLAGS)

C_SRCS=		gemtc_api.c\
		master_proc.c\
		mtc_queue.c\
		mtc_apps/mtc_apps.c # kinda hack-ish

C_HDRS=		gemtc_api.h\
		master_proc.h\
		mtc_queue.h\
		mtc_apps/mtc_apps.h # kinda hack-ish

C_OBJECTS:=	$(C_SRCS:%.c=%.o)


$(C_SRCS): %.c:

$(C_OBJECTS): %.o: %.c $(C_HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<

objs: $(C_OBJECTS)

$(SO): $(C_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(SO) $(C_OBJECTS) $(LIBS)

lib: $(SO)

clean:
	rm -f $(C_OBJECTS) $(SO) drivers/*.test

# Hack-ish but should work for now
DRV_CFLAGS= -std=c11 -W -Wall -Wextra
MIDWAY_PATH=/software/intel/composer_xe_2015.1.133/compiler/lib/intel64/ #May have to change to the MIC directory
compile-driver: lib
	$(CC) $(DRV_CFLAGS) drivers/runner.c $(SO) -I . -o drivers/runner.test

run-driver: compile-driver
	LD_LIBRARY_PATH=$(PWD):$(MIDWAY_PATH) ./drivers/runner.test

valgrind-driver: compile-driver
	LD_LIBRARY_PATH=$(PWD) valgrind ./drivers/runner.test

gdb-driver: compile-driver
	LD_LIBRARY_PATH=$(PWD) gdb ./drivers/runner.test

compile-sc: lib
	$(CC) -Wall -Wextra -g client_server/server.c $(SO) -I . -o client_server/server -lpthread
	$(CC) -Wall -Wextra -g client_server/sleep_client.c $(SO) -I . -o client_server/sleep_client
	$(CC) -Wall -Wextra -g client_server/shutdown_client.c $(SO) -I . -o client_server/shutdown_client
	$(CC) -Wall -Wextra -g client_server/dequeue_client.c $(SO) -I . -o client_server/dequeue_client
	$(CC) -Wall -Wextra -g client_server/matrix_client.c $(SO) -I . -o client_server/matrix_client

compile-scif: lib
	$(CC) -Wall -Wextra -g scif/server.c $(SO) -I . -o scif/server -lpthread -lscif
	$(CC) -Wall -Wextra -g scif/scif_client_helper.c scif/sleep_client.c $(SO) -I . -o scif/sleep_client -lscif
	$(CC) -Wall -Wextra -g scif/scif_client_helper.c scif/shutdown_client.c $(SO) -I . -o scif/shutdown_client -lscif
	$(CC) -Wall -Wextra -g scif/scif_client_helper.c scif/dequeue_client.c $(SO) -I . -o scif/dequeue_client -lscif
	$(CC) -Wall -Wextra -g scif/scif_client_helper.c scif/matrix_client.c $(SO) -I . -o scif/matrix_client -lscif
