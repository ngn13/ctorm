#pragma once

#include <stdbool.h>
#include <stdint.h>

// #######################
// ## app configuration ##
// #######################
typedef struct {
  int      max_connections; // max parallel connection count
  bool     disable_logging; // disables request logging and the banner
  bool     handle_signal;   // disables SIGINT handler (which stops app_run())
  bool     server_header;   // disable sending the "Server: ctorm" header in the response
  bool     lock_request;    // locks threads until the request handler returns
  __time_t tcp_timeout;     // sets the TCP socket timeout for sending and receiving
  uint64_t pool_size;       // app threadpool size
} app_config_t;

void app_config_new(app_config_t *config);
