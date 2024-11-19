TARGET_1 = storage-threads-mut
SRCS_1 = storage-mut.c storage-threads-mut.c

TARGET_2 = storage-threads-spe
SRCS_2 = storage-spe.c storage-threads-spe.c

TARGET_3 = storage-threads-rea
SRCS_3 = storage-rea.c storage-threads-rea.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2} ${TARGET_3}

${TARGET_1}: storage-mut.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

${TARGET_2}: storage-spe.h ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2} ${LIBS} -o ${TARGET_2}

${TARGET_3}: storage-rea.h ${SRCS_3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_3} ${LIBS} -o ${TARGET_3}

clean:
	${RM} -f *.o ${TARGET_1} ${TARGET_2}