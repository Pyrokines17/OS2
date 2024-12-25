TARGET_1 = storage-threads-mut
SRCS_1 = storage-threads-mut.c storage-mut.c depend.c compare.c swap.c

CC=gcc
RM=rm

CFLAGS= -g -Wall
INCLUDE_DIR="."
LIBS=-lpthread

all: ${TARGET_1}

${TARGET_1}: storage-mut.h depend.h compare.h swap.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}