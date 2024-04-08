#pragma once
#include "ctorm.h"
#include "util.h"
#include <netinet/in.h>
#include <stdbool.h>

#define THREADS 50
typedef int t_socketop[2];
bool start_socket(app_t *, char *, int);
