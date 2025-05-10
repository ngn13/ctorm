#pragma once
#include <stdint.h>
#include <errno.h>

/*!

 * @brief Error codes

 * Custom error numbers (errno) for ctorm library

*/
typedef enum {
  CTORM_ERR_BAD_TCP_TIMEOUT = 9900,
  CTORM_ERR_BAD_POOL_SIZE,
  CTORM_ERR_BAD_PORT,
  CTORM_ERR_BAD_HOSTNAME,
  CTORM_ERR_BAD_PATH,
  CTORM_ERR_BAD_MAX_CONN_COUNT,
  CTORM_ERR_BAD_RESPONSE_CODE,
  CTORM_ERR_BAD_CONTENT_TYPE,
  CTORM_ERR_BAD_BUFFER,

  CTORM_ERR_BAD_APP_PTR,
  CTORM_ERR_BAD_URI_PTR,
  CTORM_ERR_BAD_JSON_PTR,
  CTORM_ERR_BAD_FMT_PTR,
  CTORM_ERR_BAD_PATH_PTR,
  CTORM_ERR_BAD_DATA_PTR,
  CTORM_ERR_BAD_HEADER_PTR,
  CTORM_ERR_BAD_LOCAL_PTR,
  CTORM_ERR_BAD_QUERY_PTR,
  CTORM_ERR_BAD_PARAM_PTR,
  CTORM_ERR_BAD_CONFIG_PTR,
  CTORM_ERR_BAD_HOST_PTR,

  CTORM_ERR_PORT_TOO_LARGE,
  CTORM_ERR_HOSTNAME_TOO_LARGE,

  CTORM_ERR_POOL_FAIL,
  CTORM_ERR_LISTEN_FAIL,
  CTORM_ERR_SOCKET_OPT_FAIL,
  CTORM_ERR_FCNTL_FAIL,
  CTORM_ERR_ALLOC_FAIL,
  CTORM_ERR_READ_FAIL,
  CTORM_ERR_MUTEX_FAIL,
  CTORM_ERR_SEEK_FAIL,

  CTORM_ERR_NOT_EXISTS,
  CTORM_ERR_NO_READ_PERM,
  CTORM_ERR_NO_JSON_SUPPORT,
  CTORM_ERR_EMPTY_BODY,

  CTORM_ERR_UNKNOWN
} ctorm_error_t;

/*!

 * @brief     Get an errors description by it's error number
 * @param[in] error: Error number
 * @return    Error description

*/
const char *ctorm_error_from(int error);

/*!

 * @brief  Get current errno's description
 * @return Description for the current errno

*/
#define ctorm_error() ctorm_error_from(errno)
