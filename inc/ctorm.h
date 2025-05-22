/*

 * ctorm | Simple web framework for C
 * Written by ngn (https://ngn.tf) (2025)

*/

/*!

 * @file
 * @brief Header file that contains all the application, request and response
 *        macros. Do not include any other file to use the ctorm library in your
 *        application.

 * @section LICENSE
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once
#define CTORM_EXPORT

#include "app.h"
#include "req.h"
#include "res.h"
#include "log.h"
#include "error.h"

#define CTORM_VERSION "1.8.1" /// ctorm version number

/*!

 * Create a route that will handle all HTTP method requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define ALL(app, path, func) ctorm_app_add(app, -1, path, func)

/*!

 * Create a route that will handle all HTTP GET requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define GET(app, path, func) ctorm_app_add(app, CTORM_HTTP_GET, path, func)

/*!

 * Create a route that will handle all HTTP HEAD requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define HEAD(app, path, func) ctorm_app_add(app, CTORM_HTTP_HEAD, path, func)

/*!

 * Create a route that will handle all HTTP POST requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define POST(app, path, func) ctorm_app_add(app, CTORM_HTTP_POST, path, func)

/*!

 * Create a route that will handle all HTTP PUT requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define PUT(app, path, func) ctorm_app_add(app, CTORM_HTTP_PUT, path, func)

/*!

 * Create a route that will handle all HTTP DELETE requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define DELETE(app, path, func)                                                \
  ctorm_app_add(app, CTORM_HTTP_DELETE, path, func)

/*!

 * Create a route that will handle all HTTP OPTIONS requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define OPTIONS(app, path, func)                                               \
  ctorm_app_add(app, CTORM_HTTP_OPTIONS, path, func)

/*!

 * Create a route that will handle all HTTP TRACE requests for a given path

 * @param[in] app:  Web server created with @ref ctorm_app_new
 * @param[in] path: Route path
 * @param[in] func: Route handler function, see @ref ctorm_route_t

*/
#define TRACE(app, path, func) ctorm_app_add(app, CTORM_HTTP_TRACE, path, func)

//! Macro for @ref ctorm_req_method
#define REQ_METHOD() ctorm_req_method(req)

//! Macro for @ref ctorm_req_body
#define REQ_BODY(buffer, size) ctorm_req_body(req, buffer, size)

//! Macro for @ref ctorm_req_get
#define REQ_GET(header) ctorm_req_get(req, header)

//! Macro for @ref ctorm_req_query
#define REQ_QUERY(query) ctorm_req_query(req, query)

//! Macro for @ref ctorm_req_param
#define REQ_PARAM(param) ctorm_req_param(req, param)

//! Macro for @ref ctorm_req_local
#define REQ_LOCAL(...) ctorm_req_local(req, ##__VA_ARGS__, NULL)

//! Macro for @ref ctorm_req_form
#define REQ_FORM() ctorm_req_form(req)

//! Macro for @ref ctorm_req_json
#define REQ_JSON() ctorm_req_json(req)

//! Macro for @ref ctorm_req_ip
#define REQ_IP(buf) ctorm_req_ip(req, buf)

//! Macro for @ref ctorm_req_addr
#define REQ_ADDR() ctorm_req_addr(req)

//! Cancel a HTTP request so it doesn't get processed by the next routes
#define REQ_CANCEL() (req->cancel = true)

//! Get the HTTP request body size
#define REQ_BODY_SIZE() (req->body_size < 0 ? 0 : req->body_size)

//! Macro for @ref ctorm_res_code
#define RES_CODE(code) ctorm_res_code(res, code)

/*!

 * Copy NULL terminated character buffer to the response body. Same as calling
 * @ref ctorm_res_body with 0 as the size

 * @param[in] str: NULL terminated character buffer
 * @return    Amount of copied bytes

*/
#define RES_BODY(str) ctorm_res_body(res, str, 0)

//! Macro for @ref ctorm_res_file
#define RES_FILE(path) ctorm_res_file(res, path)

//! Macro for @ref ctorm_res_set
#define RES_SET(header, value) ctorm_res_set(res, header, value)

//! Macro for @ref ctorm_res_del
#define RES_DEL(header) ctorm_res_del(res, header)

//! Macro for @ref ctorm_res_json
#define RES_JSON(json) ctorm_res_json(res, json)

//! Macro for @ref ctorm_res_clear
#define RES_CLEAR() ctorm_res_clear(res)

//! Macro for @ref ctorm_res_fmt
#define RES_FMT(fmt, ...) ctorm_res_fmt(res, fmt, __VA_ARGS__)

//! Macro for @ref ctorm_res_add
#define RES_ADD(fmt, ...) ctorm_res_add(res, fmt, __VA_ARGS__)

//! Macro for @ref ctorm_res_redirect
#define RES_REDIRECT(uri) ctorm_res_redirect(res, uri)
