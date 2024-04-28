#pragma once
#include "ctorm.h"
#include "util.h"
#include <netinet/in.h>
#include <stdbool.h>

#define THREADS 30
typedef struct socket_args_t {
  app_t *app;
  int socket;
} socket_args_t;

bool socket_start(app_t *, char *, int);
