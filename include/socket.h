#pragma once
#include "ctorm.h"
#include "util.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct socket_args_t {
  app_t          *app;
  int             socket;
  struct sockaddr addr;
} socket_args_t;

bool socket_start(app_t *, char *, uint16_t);
