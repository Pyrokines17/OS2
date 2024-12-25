TARGET_1 = http-proxy
SRCS_1 = main.c ./thread_poll/thread_poll.c ./proxy/proxy.c ./proxy/network/network.c ./http_parser/custom_parser.c ./http_parser/source/http_parser.c ./hashmap/hashmap.c ./cache/cache.c

CC=gcc
RM=rm

CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: ./thread_poll/thread_poll.h ./proxy/proxy.h ./proxy/network/network.h ./http_parser/custom_parser.h ./http_parser/source/http_parser.h ./hashmap/hashmap.h ./cache/cache.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}