#include "config.h"
#include "error.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

ctorm_config_t *ctorm_config_new(ctorm_config_t *config) {
  if (NULL == config)
    config = calloc(1, sizeof(*config));

  if (NULL == config) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  // set the default values
  config->max_connections = 1000;
  config->disable_logging = false;
  config->handle_signal   = true;
  config->server_header   = true;
  config->lock_request    = true;
  config->tcp_timeout     = 10;
  config->pool_size       = 30;

  return config;
}

bool ctorm_config_check(ctorm_config_t *config) {
  if (NULL == config) {
    errno = CTORM_ERR_BAD_CONFIG_PTR;
    return false;
  }

  if (config->tcp_timeout < 0) {
    errno = CTORM_ERR_BAD_TCP_TIMEOUT;
    return false;
  }

  else if (config->tcp_timeout == 0)
    warn("setting the TCP timeout to 0 may allow attackers to DoS your "
         "application");

  if (config->max_connections <= 0) {
    errno = CTORM_ERR_BAD_MAX_CONN_COUNT;
    return false;
  }

  if (config->pool_size <= 0) {
    errno = CTORM_ERR_BAD_POOL_SIZE;
    return false;
  }

  return true;
}
