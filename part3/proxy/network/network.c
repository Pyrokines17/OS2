#include "network.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define DEFAULT_PORT 80
#define LISTEN_COUNT 10
#define SERVER_IP "127.0.0.1"

int safe_close1(int fd) {
    errno = SUCCESS;
    int stat = close(fd);

    if (stat == FAILURE) {
        fprintf(stderr, "Error: close failed: %s\n", strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
}

void ignore_sigpipe() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
}

int init_epoll() {
    errno = SUCCESS;
    int epoll_fd = epoll_create1(0);
    
    if (errno != SUCCESS) {
        fprintf(stderr, "Error: epoll_create1 failed: %s\n", strerror(errno));
        return FAILURE;
    }
    
    return epoll_fd;
}

int add_in_epoll(int epoll_fd, int sock_fd, struct epoll_event *event) {
    event->events = EPOLLIN | EPOLLET;
    event->data.fd = sock_fd;
    errno = SUCCESS;

    int stat = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, event);

    if (stat == FAILURE) {
        fprintf(stderr, "Error: epoll_ctl failed: %s\n", strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
}

int remove_from_epoll(int epoll_fd, int sock_fd) {
    int stat = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, NULL);

    if (stat == FAILURE) {
        fprintf(stderr, "Error: epoll_ctl failed: %s\n", strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
}

int send_message(char* buf, unsigned int len, int sock_fd) {
    int sent = 0;
    int n = 0;

    errno = SUCCESS;

    while (sent < len) {
        n = send(sock_fd, (const void*)&buf[sent], (len - sent) * sizeof(char), MSG_NOSIGNAL);

        if (n <= 0 && errno != SUCCESS) {
            fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
            return CLOSE_CONNECTION;
        }

        sent += n;
    }

    return SUCCESS;
}

int connect_to_server(char* host) {
    struct hostent *server = gethostbyname(host);

    if (server == NULL) {
        fprintf(stderr, "Error: gethostbyname failed: %s\n", strerror(h_errno));
        return FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    memcpy(&server_addr.sin_addr, server->h_addr, server->h_length);

    errno = SUCCESS;
    int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock_fd == FAILURE) {
        fprintf(stderr, "Error: socket failed: %s\n", strerror(errno));
        return FAILURE;
    }

    errno = SUCCESS;
    int stat = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));

    if (stat == FAILURE) {
        fprintf(stderr, "Error: connect failed: %s\n", strerror(errno));
        return FAILURE;
    }

    return sock_fd;
}

int create_server_socket(int port) {
    errno = SUCCESS;
    int opt = 1;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (errno != SUCCESS) {
        fprintf(stderr, "Error: socket failed: %s\n", strerror(errno));
        return FAILURE;
    }

    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    errno = SUCCESS;

    int stat = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (stat != SUCCESS) {
        fprintf(stderr, "Error: bind failed: %s\n", strerror(errno));
        return FAILURE;
    }

    errno = SUCCESS;

    stat = listen(sock_fd, LISTEN_COUNT);

    if (stat != SUCCESS) {
        fprintf(stderr, "Error: listen failed: %s\n", strerror(errno));
        safe_close1(sock_fd);
        return FAILURE;
    }

    return sock_fd;
}

void close_connection(int sock_fd, int epoll_fd) {
    errno = SUCCESS;

    int stat = remove_from_epoll(epoll_fd, sock_fd);

    if (stat == FAILURE) {
        fprintf(stderr, "Error: remove_from_epoll failed: %s\n", strerror(errno));
    }

    stat = close(sock_fd);

    if (stat == FAILURE) {
        fprintf(stderr, "Error: close failed: %s\n", strerror(errno));
    }
}
