#include "source/http_parser.h"

#define READY 1
#define NOT_READY 0

typedef struct header_t {
    char* name;
    char* value;
} header_t;

typedef struct http_message_t {
    unsigned int method;
    char* url;
    int status;
    header_t* headers;
    int headers_count;
    char* body;
    char* buf;
    size_t* len;
    int state;
} http_message_t;

void init_http_parser(http_parser* parser, http_parser_settings* settings);
http_message_t* create_http_message();
void free_http_message(http_message_t* message);
void print_http_message(const http_message_t* message);
void refresh_http_message(http_message_t* message);
