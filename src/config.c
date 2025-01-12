#include "config.h"
#include <stdlib.h>

void ctorm_config_new(ctorm_config_t *config) {
  if (NULL == config)
    return;

  config->max_connections = 1000;
  config->disable_logging = false;
  config->handle_signal   = true;
  config->server_header   = true;
  config->lock_request    = true;
  config->tcp_timeout     = 10;
  config->pool_size       = 30;
}
