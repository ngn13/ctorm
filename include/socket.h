#pragma once
#include "ctorm.h"
#include "util.h"
#include <netinet/in.h>
#include <stdbool.h>

typedef struct socket_args_t {
  app_t           *app;
  int              socket;
  struct sockaddr *address;
} socket_args_t;

bool socket_start(app_t *, char *, int);
