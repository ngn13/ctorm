#pragma once
#include "app.h"

#include <stdint.h>
#include <errno.h>

/*!

 * @brief Error codes

 * Custom error numbers (errno) for ctorm library

*/
typedef enum {
  CTORM_ERR_BAD_TCP_TIMEOUT = 9900,
  CTORM_ERR_BAD_POOL_SIZE,
  CTORM_ERR_BAD_MAX_CONN_COUNT,
  CTORM_ERR_BAD_RESPONSE_CODE,
  CTORM_ERR_BAD_CONTENT_TYPE,
  CTORM_ERR_BAD_BUFFER,
  CTORM_ERR_BAD_URI_FORMAT,
  CTORM_ERR_BAD_SCHEME,
  CTORM_ERR_BAD_AUTHORITY,
  CTORM_ERR_BAD_USERINFO,
  CTORM_ERR_BAD_HOST,
  CTORM_ERR_BAD_PORT,
  CTORM_ERR_BAD_PATH,
  CTORM_ERR_BAD_QUERY,

  CTORM_ERR_BAD_APP_PTR,
  CTORM_ERR_BAD_ADDR_PTR,
  CTORM_ERR_BAD_JSON_PTR,
  CTORM_ERR_BAD_FMT_PTR,
  CTORM_ERR_BAD_DATA_PTR,
  CTORM_ERR_BAD_HEADER_PTR,
  CTORM_ERR_BAD_LOCAL_PTR,
  CTORM_ERR_BAD_QUERY_PTR,
  CTORM_ERR_BAD_PARAM_PTR,
  CTORM_ERR_BAD_CONFIG_PTR,
  CTORM_ERR_BAD_URI_PTR,
  CTORM_ERR_BAD_AUTHORITY_PTR,
  CTORM_ERR_BAD_HOST_PTR,
  CTORM_ERR_BAD_PATH_PTR,

  CTORM_ERR_SCHEME_TOO_LARGE,
  CTORM_ERR_USERINFO_TOO_LARGE,
  CTORM_ERR_PORT_TOO_LARGE,
  CTORM_ERR_HOST_TOO_LARGE,
  CTORM_ERR_PATH_TOO_LARGE,
  CTORM_ERR_QUERY_TOO_LARGE,
  CTORM_ERR_QUERY_KEY_TOO_LARGE,
  CTORM_ERR_QUERY_VALUE_TOO_LARGE,

  CTORM_ERR_POOL_FAIL,
  CTORM_ERR_LISTEN_FAIL,
  CTORM_ERR_SOCKET_OPT_FAIL,
  CTORM_ERR_FCNTL_FAIL,
  CTORM_ERR_ALLOC_FAIL,
  CTORM_ERR_READ_FAIL,
  CTORM_ERR_MUTEX_FAIL,
  CTORM_ERR_SEEK_FAIL,
  CTORM_ERR_JSON_FAIL,
  CTORM_ERR_RESOLVE_FAIL,
  CTORM_ERR_SOCKET_FAIL,
  CTORM_ERR_BIND_FAIL,
  CTORM_ERR_ACCEPT_FAIL,

  CTORM_ERR_NOT_EXISTS,
  CTORM_ERR_NO_READ_PERM,
  CTORM_ERR_NO_JSON_SUPPORT,
  CTORM_ERR_EMPTY_BODY,
  CTORM_ERR_EMPTY_QUERY,

  CTORM_ERR_UNKNOWN
} ctorm_error_t;

/*!

 * @brief     Get an errors description by it's error number
 * @param[in] error: Error number
 * @return    Error description

*/
const char *ctorm_error_str(int error);

/*!

 * @brief     Get details about the current errno
 * @param[in] app: @ref ctorm_app_t related to the error
 * @return    Description for the detail errno

*/
const char *ctorm_error_details(ctorm_app_t *app);

/*!

 * @brief  Get current errno's description
 * @return Description for the current errno

*/
#define ctorm_error() ctorm_error_str(errno)

/*!

 * @brief  Get details about the current errno, macro for @ref
 *         ctorm_error_details
 * @return Description for the detail errno

*/
#define ctorm_details() ctorm_error_details(app)

#ifndef CTORM_EXPORT

void ctorm_error_set(ctorm_app_t *app, int error);
void ctorm_error_clear(ctorm_app_t *app);

#endif
