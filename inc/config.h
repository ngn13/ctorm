/*!

 * @file
 * @brief Header file for the web server configuration functions and definitions

*/
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*!

 * @brief Web server application configuration

 * Type for the web server application configuration, you can initialize a
 * new configuration with the default values using @ref ctorm_config_new

*/
typedef struct {
  int      max_connections; /// max parallel connection count
  bool     disable_logging; /// disables request logging and the banner
  bool     handle_signal;   /// disables SIGINT handler (which stops app_run())
  bool     server_header;   /// disable sending the "Server: ctorm" header in the response
  bool     lock_request;    /// locks threads until the request handler returns
  __time_t tcp_timeout;     /// sets the TCP socket timeout for sending and receiving
  uint64_t pool_size;       /// app threadpool size
} ctorm_config_t;

/*!

 * Initialize a new web server application configuration with the default values,
 * you should define a configuration using @ref ctorm_config_t type first and pass
 * it's address to this function

 * @param[out] config Pointer of the configuration to initialize

*/
void ctorm_config_new(ctorm_config_t *config);
