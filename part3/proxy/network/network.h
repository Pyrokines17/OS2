#include "../../hashmap/hashmap.h"
#include <sys/epoll.h>

#define SUCCESS 0
#define FAILURE -1
#define CLOSE_CONNECTION -2

void ignore_sigpipe();
int init_epoll();
int add_in_epoll(int epoll_fd, int sock_fd, struct epoll_event *event);
int remove_from_epoll(int epoll_fd, int sock_fd);
int send_message(char* buf, unsigned int len, int sock_fd);
int connect_to_server(char* host);
int create_server_socket(int port);
void close_connection(int sock_fd, int epoll_fd);
