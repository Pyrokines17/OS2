#define _GNU_SOURCE

#pragma once

#include <stdio.h>
#include <pthread.h>

#include "depend.h"

void* increase(void* arg);
void* decrease(void* arg);
void* equal(void* arg);
