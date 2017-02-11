#
# Top-lelve variables
#

INSTALL_DIR = /opt
LOCAL_DIR   = local

#
# C compilation relevant variables
#

CC     = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Wpedantic -g
OFLAGS = -c
LFLAGS = -shared -mimpure-text

LIB_DIR   = libsdb
LIB_FLAGS = -lproc

LUA_FLAGS = -llua

SDB_DIR   = sdb

#
# Top-level rules
#

compile:  ${LIB_DIR}/libsdb.so

clean:
	rm -f ${LIB_DIR}/*.o ${LIB_DIR}/*.so

help:
	@echo "Makefile options:"
	@echo "make clean       - clean object files from C bindings."
	@echo "make compile     - compiles C bindings."
	@echo "make help        - outputs this message."
	@echo "make install     - installs sdb in ${INSTALL_DIR}."
	@echo "make local       - installs sdb within the repo."
	@echo "make test        - runs test suite."
	@echo "make uninstall   - uninstall local and global installation."

install: local
	cp -r ${LOCAL_DIR} ${INSTALL_DIR}/sdb
	@echo "To use it add ${INSTALL_DIR}/sdb/bin to your PATH"

local: compile
	mkdir -p ${LOCAL_DIR}/lib
	cp ${LIB_DIR}/libsdb.so ${LOCAL_DIR}/lib/libsdb.so
	cp -r share ${LOCAL_DIR}/share
	cp -r ${SDB_DIR} ${LOCAL_DIR}/share/${SDB_DIR}
	mkdir -p ${LOCAL_DIR}/bin
	cp bin/sdb ${LOCAL_DIR}/bin/sdb

test:
	@echo "Ain't nobody got time for that!"

uninstall:
	rm -rf ${LOCAL_DIR}
	rm -rf ${INSTALL_DIR}/sdb

#
# C bindings compilation rules
#

${LIB_DIR}/libsdb.so: ${LIB_DIR}/libsdb.o ${LIB_DIR}/libsdb_lwrap.o
	${CC} ${CFLAGS} ${LFLAGS} -o $@ $^ ${LIB_FLAGS} ${LUA_FLAGS}

${LIB_DIR}/libsdb_lwrap.o: ${LIB_DIR}/libsdb_lwrap.c ${LIB_DIR}/libsdb.o
	${CC} ${CFLAGS} ${OFLAGS} $< ${LUA_FLAGS} -o $@

${LIB_DIR}/libsdb.o: ${LIB_DIR}/libsdb.c  ${LIB_DIR}/libsdb.h
	${CC} ${CFLAGS} ${OFLAGS} $< ${LIB_FLAGS} -o $@

