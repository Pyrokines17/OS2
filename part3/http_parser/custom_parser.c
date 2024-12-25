#include "custom_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEADERS 40

int on_url(http_parser* parser, const char* at, size_t length) {
    http_message_t* message = (http_message_t*)parser->data;
    message->url = (char*)malloc(length + 1);

    if (message->url == NULL) {
        fprintf(stderr, "Error allocating memory for URL\n");
        return EXIT_FAILURE;
    }

    memcpy(message->url, at, length);
    message->url[length] = '\0';
    return EXIT_SUCCESS;
}

int on_header_name(http_parser* parser, const char* at, size_t length) {
    http_message_t* message = (http_message_t*)parser->data;
    message->headers[message->headers_count].name = (char*)malloc(length + 1);

    if (message->headers[message->headers_count].name == NULL) {
        fprintf(stderr, "Error allocating memory for header name\n");
        return EXIT_FAILURE;
    }

    memcpy(message->headers[message->headers_count].name, at, length);
    message->headers[message->headers_count].name[length] = '\0';
    return EXIT_SUCCESS;
}

int on_header_value(http_parser* parser, const char* at, size_t length) {
    http_message_t* message = (http_message_t*)parser->data;
    message->headers[message->headers_count].value = (char*)malloc(length + 1);

    if (message->headers[message->headers_count].value == NULL) {
        fprintf(stderr, "Error allocating memory for header value\n");
        return EXIT_FAILURE;
    }

    memcpy(message->headers[message->headers_count].value, at, length);
    message->headers[message->headers_count].value[length] = '\0';
    message->headers_count++;
    return EXIT_SUCCESS;
}

int on_body(http_parser* parser, const char* at, size_t length) {
    http_message_t* message = (http_message_t*)parser->data;
    message->body = (char*)malloc(length + 1);

    if (message->body == NULL) {
        fprintf(stderr, "Error allocating memory for body\n");
        return EXIT_FAILURE;
    }

    memcpy(message->body, at, length);
    return EXIT_SUCCESS;
}

int on_status(http_parser* parser, const char* at, size_t length) {
    http_message_t* message = (http_message_t*)parser->data;
    message->status = parser->status_code;
    return EXIT_SUCCESS;
}

int on_message_complete(http_parser* parser) {
    http_message_t* message = (http_message_t*)parser->data;
    message->state = READY;
    return EXIT_SUCCESS;
}

void init_http_parser(http_parser* parser, http_parser_settings* settings) {
    http_parser_init(parser, HTTP_BOTH);
    http_parser_settings_init(settings);
    settings->on_url = on_url;
    settings->on_header_field = on_header_name;
    settings->on_header_value = on_header_value;
    settings->on_body = on_body;
    settings->on_status = on_status;
    settings->on_message_complete = on_message_complete;
}

http_message_t* create_http_message() {
    http_message_t* message = (http_message_t*)malloc(sizeof(http_message_t));
    if (message == NULL) {
        return NULL;
    }

    message->headers_count = 0;
    message->headers = (header_t*)malloc(MAX_HEADERS * sizeof(header_t));
    
    if (message->headers == NULL) {
        fprintf(stderr, "Error allocating memory for headers\n");
        free(message);
        return NULL;
    }

    for (int i = 0; i < MAX_HEADERS; i++) {
        message->headers[i].name = NULL;
        message->headers[i].value = NULL;
    }

    message->body = NULL;
    message->url = NULL;
    message->state = NOT_READY;

    return message;
}

void free_http_message(http_message_t* message) {
    if (message == NULL) {
        return;
    }

    if (message->url != NULL) {
        free(message->url);
    }

    if (message->body != NULL) {
        free(message->body);
    }

    for (int i = 0; i < message->headers_count; i++) {
        if (message->headers[i].name != NULL) {
            free(message->headers[i].name);
        }
        if (message->headers[i].value != NULL) {
            free(message->headers[i].value);
        }
    }

    free(message->headers);
    free(message);
}

void print_http_message(const http_message_t* message) {
    if (message == NULL) {
        return;
    }

    printf("\n --- HTTP MESSAGE --- \n");
    printf("Method: %d\n", message->method);
    printf("URL: %s\n", message->url);
    printf("Headers:\n");

    for (int i = 0; i < message->headers_count; i++) {
        printf("%s: %s\n", message->headers[i].name, message->headers[i].value);
    }

    if (message->body != NULL && strlen(message->body) > 0) {
        printf("Body: %s\n", message->body);
    } else {
        printf("Body: Empty\n");
    }

    printf("Status: %d\n", message->status);
    printf("State: %d\n", message->state);
    printf(" --- END --- \n");
}

void refresh_http_message(http_message_t* message) {
    if (message == NULL) {
        return;
    }

    if (message->url != NULL) {
        free(message->url);
        message->url = NULL;
    }

    if (message->body != NULL) {
        free(message->body);
        message->body = NULL;
    }

    for (int i = 0; i < message->headers_count; i++) {
        if (message->headers[i].name != NULL) {
            free(message->headers[i].name);
            message->headers[i].name = NULL;
        }
        if (message->headers[i].value != NULL) {
            free(message->headers[i].value);
            message->headers[i].value = NULL;
        }
    }

    message->headers_count = 0;
}
