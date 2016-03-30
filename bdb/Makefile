CC       ?= gcc
CFLAGS   += -Wall -Wextra -std=c11 -Wpedantic -g
COPT_LVL ?= -O2

SRCDIR    = src

C_SRCS    = ${SRCDIR}/bdb_utils.c   \
	    ${SRCDIR}/bdb_session.c \
	    ${SRCDIR}/bdb_main.c

C_HDRS    = ${SRCDIR}/bdb_utils.h   \
	    ${SRCDIR}/bdb_session.h

C_OBJECTS = ${C_SRCS:%.c=%.o}

PROG      = bdb

all: ${PROG}

${PROG}: ${C_OBJECTS}
	${CC} ${CFLAGS} ${COPT_LVL} -o ${PROG} ${C_OBJECTS}

.c.o:
	${CC} ${CFLAGS} ${COPT_LVL} -o $@ -c $<

# One header change recompiles all C files :-S
${C_OBJECTS}: ${C_HDRS}

clean:
	rm -f ${C_OBJECTS} ${PROG}
