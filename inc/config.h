#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/*!

 * @brief Web server application configuration

 * Type for the web server application configuration, you can initialize a
 * new configuration with the default values using @ref ctorm_config_new

*/
typedef struct {
  bool     disable_logging; /// disables request logging and the banner
  bool     handle_signal;   /// disables SIGINT handler (which stops app_run())
  bool     server_header;   /// disable the "Server: ctorm" header
  bool     lock_request;    /// locks threads until the request handler returns
  time_t   tcp_timeout; /// TCP socket timeout for sending and receiving data
  uint32_t max_connections; /// max parallel connection count
  uint32_t pool_size;       /// app threadpool size
} ctorm_config_t;

/*!

 * Initializes the provided @ref ctorm_config_t configuration with the default
 * values. If no configuration is specified, function allocates a new
 * configuration and returns it. In this case you should free() this pointer
 * after you are done with it

 * @param[out] config: Pointer to the configuration to initialize

*/
ctorm_config_t *ctorm_config_new(ctorm_config_t *config);

/*!

 * Checks if the provided @ref ctorm_config_t configuration is valid. This
 * function is also called by @ref ctorm_app_new, so you do not need to call
 * this function if you intend to use the configuration for creating a web
 * server.

 * @param[in] config: Pointer to the configuration to check

*/
bool ctorm_config_check(ctorm_config_t *config);
