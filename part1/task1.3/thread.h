#pragma once

#define CREATE_ERROR 1
#define JOIN_ERROR 2
#define INIT_ATTR_ERROR 3
#define SET_ATTR_ERROR 4
#define DESTROY_ATTR_ERROR 5

typedef struct {
    int firstVar;
    char* secondVar;
} myStruct;

void* printStruct(void* arg);
