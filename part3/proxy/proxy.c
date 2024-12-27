#define _GNU_SOURCE

#include "../http_parser/custom_parser.h"
#include "network/network.h"
#include "proxy.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>

char* bad_gateway = "HTTP/1.1 502 Bad Gateway\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 112\r\n"
                    "\r\n"
                    "<html>\n"
                    "<head><title>502 Bad Gateway</title></head>\n"
                    "<body>\n"
                    "<h1>Bad Gateway</h1>\n"
                    "<p>The proxy was unable to connect to the server.</p>\n"
                    "</body>\n"
                    "</html>\n";

char* bad_timeout = "HTTP/1.1 504 Gateway Timeout\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 116\r\n"
                    "\r\n"
                    "<html>\n"
                    "<head><title>504 Gateway Timeout</title></head>\n"
                    "<body>\n"
                    "<h1>Gateway Timeout</h1>\n"
                    "<p>The server did not respond within the specified time.</p>\n"
                    "</body>\n"
                    "</html>\n";

typedef struct thread_args_t {
    proxy_t* proxy;
    int epoll_fd;
} thread_args_t;

int continue_work = TRUE;

void* handle_client(void* arg);

int safe_close(int fd) {
    int stat = close(fd);

    if (stat == FAILURE) {
        fprintf(stderr, "Failed to close socket\n");
    }

    return stat;
}

int safe_mutex_unlock(pthread_mutex_t* mutex) {
    int stat = pthread_mutex_unlock(mutex);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(stat));
    }

    return stat;
}

proxy_t* proxy_create() {
    proxy_t* proxy = (proxy_t*)malloc(sizeof(proxy_t));
    
    if (proxy == NULL) {
        fprintf(stderr, "Failed to allocate memory for proxy\n");
        return NULL;
    }

    proxy->thread_poll = thread_poll_create(DEF_THREADS);

    if (proxy->thread_poll == NULL) {
        fprintf(stderr, "Failed to create thread poll\n");
        free(proxy);
        return NULL;
    }

    proxy->cache = cache_create(DEF_CACHE_INIT_SIZE, DEF_CACHE_MAX_SIZE, DEF_CACHE_TTL);

    if (proxy->cache == NULL) {
        fprintf(stderr, "Failed to create cache\n");
        thread_poll_destroy(proxy->thread_poll);
        free(proxy);
        return NULL;
    }

    proxy->server_socket = create_server_socket(DEF_PORT);

    if (proxy->server_socket == FAILURE) {
        fprintf(stderr, "Failed to create server socket\n");
        cache_destroy(proxy->cache);
        thread_poll_destroy(proxy->thread_poll);
        free(proxy);
        return NULL;
    }

    printf("Proxy started on port %d\n", DEF_PORT);

    return proxy;
}

void proxy_destroy(proxy_t* proxy) {
    thread_poll_destroy(proxy->thread_poll);
    printf("Thread poll destroyed\n");
    safe_close(proxy->server_socket);
    printf("Server socket closed\n");
    cache_destroy(proxy->cache);
    printf("Cache destroyed\n");
    free(proxy);
    printf("Proxy destroyed\n");
}

int check_get(char* buf, size_t* cur_len) {
    char* get = strstr(buf, "GET");
    int pos = (int)(get - buf);
    int len = BUFFER_SIZE - pos;

    if (get != NULL) {
        memcpy(buf, get, sizeof(char) * len);
        memset(&buf[len], 0, pos);
        return TRUE;
    }

    *cur_len = len;

    return FALSE;
}

char* find_locations(http_message_t* message) {
    for (int i = 0; i < message->headers_count; ++i) {
        if ((message->headers[i].name != NULL) && (strstr(message->headers[i].name, "ocation") != NULL)) {
            return message->headers[i].value;
        }
    }

    return NULL;
}

int get_content_length(http_message_t* message) {
    for (int i = 0; i < message->headers_count; ++i) {
        if (strcmp(message->headers[i].name, "Content-Length") == 0) {
            int len = atoi(message->headers[i].value);
            if (len > 0) {
                return len;
            }
            break;
        }
    }

    return -1;
}

char* mesh_transfer_data(
    char* buf, 
    int* buf_size, 
    int* curr_len, 
    http_message_t* message, 
    int server_sock,
    int client_sock,
    char* url,
    cache_t* cache) {
        int read_bytes = 0;

        errno = EXIT_SUCCESS;

        int no_deleted = TRUE;
        int answer = send_message(buf, *curr_len, client_sock);
        int stat;

        while (continue_work == TRUE) {
            read_bytes = recv(server_sock, message->buf, BUFFER_SIZE * sizeof(char), MSG_NOSIGNAL);

            if ((read_bytes <= 0) && (errno == EINPROGRESS)) {
                stat = send_message(bad_timeout, strlen(bad_timeout), client_sock);
                if (stat != EXIT_SUCCESS) {
                    fprintf(stderr, "Failed to send message in mesh_tranfer_data\n");
                }
                break;
            } else if ((read_bytes <= 0) || (errno != EXIT_SUCCESS)) {
                break;
            }

            if (answer == EXIT_SUCCESS) {
                answer = send_message(message->buf, read_bytes, client_sock);

                if (answer != EXIT_SUCCESS) {
                    fprintf(stderr, "Failed to send message in mesh_transfer_data\n");
                }
            }

            if ((buf != NULL) && (*buf_size + read_bytes < cache->max_size)) {
                if (*curr_len + read_bytes >= *buf_size) {
                    buf = (char*)realloc(buf, sizeof(char) * (*buf_size + BUFFER_SIZE));

                    if (buf == NULL) {
                        continue;
                    }
                    
                    *buf_size += BUFFER_SIZE;
                }
                
                memcpy(&buf[*curr_len], message->buf, read_bytes);
                *curr_len += read_bytes;
                memset(message->buf, 0, read_bytes);
            } else if (no_deleted == TRUE) {
                *buf_size = *buf_size + read_bytes;
                delete_resource(cache, &(resource_t){.url = url});
                no_deleted = FALSE;
            }
        }

        return buf;
}

void streaming_transfer_data(char* buf, unsigned int buf_size, int server_sock, int client_sock) {
    int read_bytes = buf_size;
    int stat;

    errno = EXIT_SUCCESS;

    while (continue_work == TRUE) {
        stat = send_message(buf, read_bytes, client_sock);

        if (stat == CLOSE_CONNECTION) {
            break;
        }

        read_bytes = recv(server_sock, buf, buf_size, MSG_NOSIGNAL);

        if ((read_bytes <= 0) && (errno == EINPROGRESS)) {
            stat = send_message(bad_timeout, strlen(bad_timeout), client_sock);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to send message in streaming_transfer_data\n");
            }
            break;
        } else if ((read_bytes <= 0) && (errno != EINPROGRESS)) {
            break;
        }
    }
}

size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

int is_succeess(int status) {
    return status >= 200 && status < 300;
}

int is_redirect(int status) {
    return status >= 300 && status < 400;
}

int caching_transfer_data(char* data, int data_size, int cur_len, int server_socket, int client_socket) {
    int stat = send_message(data, cur_len, client_socket);

    if (stat == CLOSE_CONNECTION) {
        fprintf(stderr, "Error: send failed\n");
        return EXIT_FAILURE;
    }

    int read_bytes = 0;

    errno = EXIT_SUCCESS;

    int wait_amount = min(BUFFER_SIZE, data_size - cur_len);
    int answer = EXIT_SUCCESS;

    while (continue_work == TRUE && (data_size > cur_len)) {
        read_bytes = recv(server_socket, &data[cur_len], wait_amount, MSG_NOSIGNAL);

        if ((read_bytes <= 0) && (errno == EINPROGRESS)) {
            stat = send_message(bad_timeout, strlen(bad_timeout), client_socket);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to send message in caching_transfer_data\n");
            }
            return EXIT_FAILURE;
        } else if ((read_bytes <= 0) && (errno != EINPROGRESS)) {
            return EXIT_FAILURE;
        }

        if (answer == EXIT_SUCCESS) {
            answer = send_message(&data[cur_len], read_bytes, client_socket);
        }

        cur_len += read_bytes;
        wait_amount = min(BUFFER_SIZE, data_size - cur_len);
    } 

    return EXIT_SUCCESS;
}

char* read_header(int server_socket, int* header_end, int* curr_len) {
    char* buf = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for header\n");
        return NULL;
    }

    memset(buf, 0, BUFFER_SIZE);

    int read_bytes = 0;

    errno = EXIT_SUCCESS;

    while((strstr(buf, "\r\n\r\n") == NULL) && (*curr_len < BUFFER_SIZE)) {
        read_bytes = recv(server_socket, &buf[*curr_len], BUFFER_SIZE - *curr_len, MSG_NOSIGNAL);

        if ((read_bytes <= 0) && (errno != EXIT_SUCCESS)) {
            free(buf);
            buf = NULL;
            *curr_len = 0;
            break;
        }

        *curr_len += read_bytes;
    } 
    
    if ((buf != NULL) && (strstr(buf, "\r\n\r\n") != NULL)) {
        *header_end = (int)(strstr(buf, "\r\n\r\n") - buf) + 4;
    } else if ((buf != NULL) && (strstr(buf, "\r\n\r\n") == NULL)) {
        free(buf);
        buf = NULL;
        *curr_len = 0;
    }

    return buf;
}

char* read_response(
    http_message_t* message,
    http_parser* parser,
    http_parser_settings* settings,
    cache_t* cache,
    size_t max_size, 
    int server_socket, 
    int client_socket, 
    int* answer, 
    char* url) {
        int buf_len = 0;
        int header_len = 0;

        printf("Reading response\n");

        char* header = read_header(server_socket, &header_len, &buf_len);

        if (header == NULL) {
            fprintf(stderr, "Failed to read header\n");
            *answer = FAILURE;
            free(header);
            return NULL;
        }

        http_parser_execute(parser, settings, header, BUFFER_SIZE);

        int content_len = -1;

        printf("Get response on URL %s with status %d\n", url, message->status);

        if ((message->status == READY) && is_succeess(message->status)) {
            content_len = get_content_length(message);
        } else if ((message->status == READY) && is_redirect(message->status)) {
            *answer = REDIRECT;
            free(header);
            return NULL;
        } else if (message->status == READY) {
            int stat = send_message(header, buf_len, client_socket);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to send message in read_response\n");
            }
            *answer = FAILURE;
            free(header);
            return NULL;
        }

        if ((content_len != FAILURE) && (content_len + header_len <= max_size) && (message->status != 206)) {
            header = realloc(header, sizeof(char) * (content_len + header_len));
            printf("Resource %s start in caching\n", url);

            *answer = caching_transfer_data(header, content_len + header_len, buf_len, server_socket, client_socket);

            if (*answer == SUCCESS) {
                printf("Resource %s cached\n", url);
                *answer = content_len + header_len;
            } else if (*answer == FAILURE) {
                fprintf(stderr, "Failed to cache resource\n");
            }
        } else if ((content_len == FAILURE) && (message->status != 206)) {
            printf("Resource %s start in meshing\n", url);
            header_len = BUFFER_SIZE;
            header = mesh_transfer_data(header, &header_len, &buf_len, message, server_socket, client_socket, url, cache);
            printf("Resource %s meshed\n", url);

            if (buf_len > cache->max_size) {
                delete_resource(cache, &(resource_t){.url = url});
                free(header);
                *answer = FAILURE;
            } else {
                *answer = buf_len;
            }
        } else {
            printf("Resource %s start in streaming\n", url);
            delete_resource(cache, &(resource_t){.url = url});
            streaming_transfer_data(header, buf_len, server_socket, client_socket);
            printf("Resource %s streamed\n", url);
            free(header);
            header = NULL;
            *answer = SUCCESS;
        }

        return header;
}

char* change_url(char* get_message, char* old_url, char* new_url) {
    int start = (int)(strstr(get_message, old_url) - get_message);
    char* new_gm = (char*)malloc(strlen(get_message) + strlen(new_url) - strlen(old_url) + 1);

    if (new_gm == NULL) {
        fprintf(stderr, "Failed to allocate memory for new get message\n");
        return NULL;
    }

    memcpy(new_gm, get_message, start);
    memcpy(&new_gm[start], new_url, strlen(new_url));
    memcpy(&new_gm[start + strlen(new_url)], &get_message[start + strlen(old_url)], strlen(get_message) - start - strlen(old_url));
    new_gm[strlen(get_message) + strlen(new_url) - strlen(old_url)] = '\0';
    free(get_message);
    return new_gm;
}

int get_response(
    int client_socket,
    http_parser* parser,
    http_parser_settings* settings,
    http_message_t* message,
    cache_t* cache,
    char* host,
    char* url,
    char** get_message,
    size_t len) {
        int server_socket = connect_to_server(host);

        if (server_socket == FAILURE) {
            send_message(bad_gateway, strlen(bad_gateway), client_socket);
            fprintf(stderr, "Failed to connect to server\n");
            return FAILURE;
        }

        struct timeval timeout;
        timeout.tv_sec = WAIT_RESPONSE_S;
        timeout.tv_usec = WAIT_RESPONSE_NS;

        int stat = setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

        if (stat != EXIT_SUCCESS) {
            fprintf(stderr, "Failed to set socket options\n");
            return FAILURE;
        }

        printf("Connected to host %s\n", host);

        stat = send_message(*get_message, len, server_socket);

        if (stat != EXIT_SUCCESS) {
            delete_resource(cache, &(resource_t){.url = url});
            safe_close(server_socket);
            fprintf(stderr, "Failed to send message\n");
            return FAILURE;
        }

        int answer;

        init_http_parser(parser, settings);
        refresh_http_message(message);

        char* buf = read_response(message, parser, settings, cache, cache->max_size, server_socket, client_socket, &answer, url);

        if (answer == REDIRECT) {
            char* new_url = find_locations(message);

            if ((new_url == NULL) || (strstr(new_url, "https") != NULL)) {
                answer = send_message(bad_gateway, strlen(bad_gateway), client_socket);
                if (answer != EXIT_SUCCESS) {
                    fprintf(stderr, "Failed to send message in get_response\n");
                }
                delete_resource(cache, &(resource_t){.url = url});
                safe_close(server_socket);
                return answer;
            }

            printf("Redirecting to %s\n", new_url);
            *get_message = change_url(*get_message, url, new_url);

            stat = send_message(*get_message, strlen(*get_message), server_socket);

            if (stat != EXIT_SUCCESS) {
                delete_resource(cache, &(resource_t){.url = url});
                safe_close(server_socket);
                fprintf(stderr, "Failed to send message? close connection\n");
                return CLOSE_CONNECTION;
            }

            init_http_parser(parser, settings);
            refresh_http_message(message);

            buf = read_response(message, parser, settings, cache, cache->max_size, server_socket, client_socket, &answer, new_url);
        }

        stat = close(server_socket);

        if (stat == FAILURE) {
            fprintf(stderr, "Failed to close socket\n");
        }

        if ((answer > 0) && (buf != NULL)) {
            resource_t resource;
            resource.url = url;
            resource.data = buf;
            resource.size = answer;
            resource.state = READY;
            put_resource(cache, &resource);
            free(buf);
            return SUCCESS;
        } else if (answer == REDIRECT) {
            delete_resource(cache, &(resource_t){.url = url});

            stat = send_message(bad_gateway, strlen(bad_gateway), client_socket);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to send message\n");
                safe_close(server_socket);
                return CLOSE_CONNECTION;
            }

            return FAILURE;
        } else {
            delete_resource(cache, &(resource_t){.url = url});
            return FAILURE;
        }
}

resource_t* wait_response(cache_t* cache, char* url) {
    resource_t* resource = find_resource(cache, url);

    struct timespec timeout;

    timeout.tv_sec = 100;
    timeout.tv_nsec = WAIT_RESPONSE_NS;

    int stat = pthread_mutex_lock(&cache->mutex);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to lock mutex: %s\n", strerror(stat));
        return NULL;
    }

    printf("Waiting for response on URL %s\n", url);

    while ((resource == NULL) || ((resource != NULL) && (resource->state == IN_PROGRESS))) {
        if (resource != NULL) {
            free(resource);
        }

        stat = pthread_cond_timedwait(&cache->cond, &cache->mutex, &timeout);

        if ((stat != EXIT_SUCCESS) && (stat != ETIMEDOUT)) {
            fprintf(stderr, "Failed to wait for condition: %s\n", strerror(stat));
            break;
        }

        resource = find_resource(cache, url);
    }

    printf("Response on URL %s received\n", url);

    stat = safe_mutex_unlock(&cache->mutex);

    if (stat != EXIT_SUCCESS) {
        printf("Failed to unlock mutex: %s\n", strerror(stat));
    }

    return resource;
}

int handle_client_message (
    http_parser* parser,
    http_parser_settings* settings,
    http_message_t* message,
    int socket,
    cache_t* cache) {
        int stat = check_get(message->buf, message->len);
        char* res;

        if (stat == FALSE) {
            return SUCCESS;
        }

        size_t nparsed = http_parser_execute(parser, settings, (const char*)message->buf, *message->len);

        if (nparsed != *message->len) {
            init_http_parser(parser, settings);
            return FAILURE;
        }

        if (message->state != READY) {
            init_http_parser(parser, settings);
            return FAILURE;
        }

        message->method = parser->method;
        int answer = SUCCESS;

        if (message->method == HTTP_GET) {
            resource_t* resource = find_resource(cache, message->url);

            if ((resource != NULL) && (resource->state == READY)) {
                printf("Resource %s found in cache\n", message->url);
                answer = send_message(resource->data, resource->size, socket);
                if (answer != EXIT_SUCCESS) {
                    fprintf(stderr, "Failed to send message in handle_client_message\n");
                }
                free(resource->data);
                free(resource);
            } else {
                if ((resource != NULL) && (resource->state == IN_PROGRESS)) {
                    printf("Resource %s is in progress\n", message->url);
                    free(resource);
                    return IN_PROGRESS;
                } 

                printf("Resource %s not found in cache\n", message->url);

                for (int i = 0; i < message->headers_count; ++i) {
                    res = strstr(message->headers[i].name, "Host");

                    if (res != NULL) {
                        char* get_message = (char*)malloc(*message->len + 1);
                        memcpy(get_message, message->buf, *message->len);
                        get_message[*message->len] = '\0';

                        char* url = message->url;
                        message->url = NULL;

                        set_resource_in_progress(cache, url);
                        answer = get_response(socket, parser, settings, message, cache, message->headers[i].value, url, &get_message, *message->len);
                        free(get_message);
                        break;
                    }
                }
            }
        }

        init_http_parser(parser, settings);
        return answer;
    }

int read_message(
    int socket,
    char* buf,
    http_message_t* message,
    http_parser* parser,
    http_parser_settings* settings,
    int epoll_fd,
    cache_t* cache) {
        size_t curr_len = 0;
        message->len = &curr_len;
        int answer = FAILURE;

        errno = EXIT_SUCCESS;

        while (answer == FAILURE) {
            int readb = recv(socket, &buf[curr_len], BUFFER_SIZE, 0);

            if ((readb < 0) && (errno != EXIT_SUCCESS)) {
                return CLOSE_CONNECTION;
            } else if (readb <= 0) {
                break;
            }

            curr_len += readb;
            answer = handle_client_message(parser, settings, message, socket, cache);

            if (answer == IN_PROGRESS) {
                resource_t* resource = wait_response(cache, message->url);

                if ((resource != NULL) && (resource->state == READY)) {
                    int stat = send_message(resource->data, resource->size, socket);
                    if (stat != EXIT_SUCCESS) {
                        fprintf(stderr, "Failed to send message in read_message\n");
                    }
                    break;
                }

                answer = FAILURE;
            } else if (answer == CLOSE_CONNECTION) {
                return CLOSE_CONNECTION;
            }

            refresh_http_message(message);
        }

        return SUCCESS;
}

void init_events(short* mask) {
    for (int i = 0; i < AMOUNT_EVENTS; ++i) {
        mask[i] = 0;
    }
}

int find_free_event(short* mask) {
    for (int i = 0; i < AMOUNT_EVENTS; ++i) {
        if (mask[i] == FREE) {
            return i;
        }
    }

    return FAILURE;
}

void set_free(int sock_fd, short* mask) {
    for (int i = 0; i < AMOUNT_EVENTS; ++i) {
        if (mask[i] == sock_fd) {
            mask[i] = FREE;
            break;
        }
    }
}

void exit_handler() {
    continue_work = FALSE;
}

void* handle_client(void* arg) {
    ignore_sigpipe();

    thread_args_t* args = (thread_args_t*)arg;
    proxy_t* proxy = args->proxy;
    int stat;

    http_parser parser; http_parser_settings settings;
    init_http_parser(&parser, &settings);
    http_message_t* message = create_http_message();
    parser.data = message;

    char* buf = (char*)malloc(sizeof(char) * BUFFER_SIZE);

    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        return NULL;
    }

    size_t curr_len = 0;

    message->buf = buf;
    message->len = &curr_len;

    short mask[AMOUNT_EVENTS];
    init_events(mask);
    struct epoll_event events[AMOUNT_EVENTS];
    int epoll_fd = args->epoll_fd;

    while (continue_work == TRUE) {
        int socket = get_item(proxy->thread_poll->queue);

        if (socket != FAILURE) {
            int pos = find_free_event(mask);

            if (pos >= 0) {
                add_in_epoll(epoll_fd, socket, &events[pos]);
                printf("Added socket %d to epoll\n", socket);
            } else {
                fprintf(stderr, "Failed to find free event\n");
            }
        }

        int nfds = epoll_wait(epoll_fd, events, AMOUNT_EVENTS, 500);

        if (nfds == FAILURE) {
            fprintf(stderr, "Failed to wait for events in thread %d: %s\n", gettid(), strerror(errno));
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].events & EPOLLIN) {
                stat = read_message(events[i].data.fd, buf, message, &parser, &settings, epoll_fd, proxy->cache);

                if (stat == CLOSE_CONNECTION) {
                    set_free(events[i].data.fd, mask);
                    close_connection(events[i].data.fd, epoll_fd);
                }

                curr_len = 0;

                memset(buf, 0, BUFFER_SIZE);
                refresh_http_message(message);
            } else {
                set_free(events[i].data.fd, mask);
                remove_from_epoll(epoll_fd, events[i].data.fd);
            }
        }
    }

    free(buf);
    free_http_message(message);
    
    stat = close(epoll_fd);

    if (stat == FAILURE) {
        fprintf(stderr, "Failed to close epoll\n");
    }

    printf("Thread %d finished\n", gettid());

    return NULL;
}

void proxy_start(proxy_t* proxy) {
    ignore_sigpipe();

    printf("Proxy started on pid %d\n", getpid());

    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);

    errno = EXIT_SUCCESS;
    int stat;

    thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t) * proxy->thread_poll->nthreads);

    if (args == NULL) {
        fprintf(stderr, "Failed to allocate memory for thread args\n");
        return;
    }

    for (int i = 0; i < proxy->thread_poll->nthreads; ++i) {
        args[i].proxy = proxy;
        args[i].epoll_fd = init_epoll();
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    stat = setsockopt(proxy->server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to set socket options: %s\n", strerror(errno));
    }    

    thread_poll_start(proxy->thread_poll, handle_client, args, sizeof(thread_args_t));

    while (continue_work == TRUE) {
        int new_fd = accept(proxy->server_socket, NULL, NULL);

        if (new_fd == FAILURE) {
            continue;
        }

        stat = add_item(proxy->thread_poll->queue, new_fd);

        if (stat != SUCCESS) {
            fprintf(stderr, "Failed to add item\n");
        }
    }

    proxy_destroy(proxy);

    free(args);
}
