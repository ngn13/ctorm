/*

 * ctorm | Simple web framework for C
 * Written by ngn (https://ngn.tf) (2025)

*/

/*!

 * @file
 * @brief Header file that contains all the application, request
 *        and response macros

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
#include "errors.h"

/*!

 * Create a route that will handle all HTTP method requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define ALL(app, path, func) ctorm_app_add(app, NULL, false, path, func)

/*!

 * Create a route that will handle all HTTP GET requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define GET(app, path, func) ctorm_app_add(app, "GET", false, path, func)

/*!

 * Create a route that will handle all HTTP PUT requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define PUT(app, path, func) ctorm_app_add(app, "PUT", false, path, func)

/*!

 * Create a route that will handle all HTTP HEAD requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define HEAD(app, path, func) ctorm_app_add(app, "HEAD", false, path, func)

/*!

 * Create a route that will handle all HTTP POST requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define POST(app, path, func) ctorm_app_add(app, "POST", false, path, func)

/*!

 * Create a route that will handle all HTTP DELETE requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define DELETE(app, path, func) ctorm_app_add(app, "DELETE", false, path, func)

/*!

 * Create a route that will handle all HTTP OPTIONS requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Route path
 * @param[in] func Route handler function, see @ref ctorm_route_t

*/
#define OPTIONS(app, path, func) ctorm_app_add(app, "OPTIONS", false, path, func)

/*!

 * Create a middleware that will handle all HTTP method requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_ALL(app, path, func) ctorm_app_add(app, NULL, true, path, func)

/*!

 * Create a middleware that will handle all HTTP GET requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_GET(app, path, func) ctorm_app_add(app, "GET", true, path, func)

/*!

 * Create a middleware that will handle all HTTP PUT requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_PUT(app, path, func) ctorm_app_add(app, "PUT", true, path, func)

/*!

 * Create a middleware that will handle all HTTP HEAD requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_HEAD(app, path, func) ctorm_app_add(app, "HEAD", true, path, func)

/*!

 * Create a middleware that will handle all HTTP POST requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_POST(app, path, func) ctorm_app_add(app, "POST", true, path, func)

/*!

 * Create a middleware that will handle all HTTP DELETE requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_DELETE(app, path, func) ctorm_app_add(app, "DELETE", true, path, func)

/*!

 * Create a middleware that will handle all HTTP OPTIONS requests
 * for a given path

 * @param[in] app  ctorm web server application
 * @param[in] path Middleware route path
 * @param[in] func Middleware route handler function, see @ref ctorm_route_t

*/
#define MIDDLEWARE_OPTIONS(app, path, func) ctorm_app_add(app, "OPTIONS", true, path, func)

/*!

 * Get request method name
 * @return HTTP method name ("GET", "POST" etc.)

*/
#define REQ_METHOD() ctorm_req_method(req)

/*!

 * Get request body size
 * @return Unread request body size, 0 if all the body
 *         is read or the request does not have a body

*/
#define REQ_BODY_SIZE() ctorm_req_body_size(req)

/*!

 * Read specific amount of bytes from the request body into a
 * specified buffer, if there are less bytes left in the request body
 * than the specified amount of bytes, then this function will read less
 * bytes into the buffer than the specified amount

 * @param[in] buffer Character buffer
 * @param[in] size   Amount of bytes to copy into the buffer
 * @return Amount of read bytes

*/
#define REQ_BODY(buffer, size) ctorm_req_body(req, buffer, size)

/*!

 * Get a request header value by name
 * @param[in] header Header name
 * @return Header value

*/
#define REQ_GET(header) ctorm_req_get(req, header)

/*!

 * Get URL query value by name
 * @param[in] query URL query name
 * @return Query value

*/
#define REQ_QUERY(query) ctorm_req_query(req, query)

/*!

 * Get URL parameter value by name
 * @param[in] param URL parameter name
 * @return Parameter value

*/
#define REQ_PARAM(param) ctorm_req_param(req, param)

/*!

 * Get or set a local
 * @param[in] name Local name
 * @param[in] ...  Local value

*/
#define REQ_LOCAL(local, ...) ctorm_req_local(req, local, ##__VA_ARGS__, NULL)

/*!

 * Parse URL form encoded request body
 * @return URL decoded body structure

*/
#define REQ_FORM() ctorm_req_form(req)

/*!

 * Parse JSON encoded request body
 * @return JSON decoded body structure

*/
#define REQ_JSON() ctorm_req_json(req)

/*!

 * Cancel a HTTP request so it doesn't get processed by
 * the next middlewares and routes

*/
#define REQ_CANCEL() (req->cancel = true)

/*!

 * Set HTTP response code
 * @param[in] _code HTTP response code

*/
#define RES_CODE(_code) (res->code = _code)

/*!

 * Copy NULL terminated character buffer to the
 * response body
 * @param[in] str NULL terminated character buffer

*/
#define RES_SEND(str) ctorm_res_send(res, str, 0)

/*!

 * Copy a file contents to the response body
 * @param[in] path File path

*/
#define RES_SENDFILE(path) ctorm_res_sendfile(res, path)

/*!

 * Set HTTP response header
 * @param[in] header HTTP header name
 * @param[in] value  HTTP header value

*/
#define RES_SET(header, value) ctorm_res_set(res, header, value)

/*!

 * Remove a HTTP response header by name
 * @param[in] header HTTP header name

*/
#define RES_DEL(header) ctorm_res_del(res, header)

/*!

 * Decode JSON body and copy it to the response body
 * @param[in] json JSON encoded data structure

*/
#define RES_JSON(json) ctorm_res_json(res, json)

/*!

 * Clear (empty) the response body

*/
#define RES_CLEAR() ctorm_res_clear(res)

/*!

 * Set response body to a formatted string
 * @param[in] fmt Format string
 * @param[in] ... Arguments for the format string

*/
#define RES_FMT(fmt, ...) ctorm_res_fmt(res, fmt, __VA_ARGS__)

/*!

 * Append formatted string to the response body
 * @param[in] fmt Format string
 * @return Format string arguments

*/
#define RES_ADD(fmt, ...) ctorm_res_add(res, fmt, __VA_ARGS__)

/*!

 * Redirect response to another URL using the location
 * header
 * @param[in] url Redirect URL

*/
#define RES_REDIRECT(url) ctorm_res_redirect(res, url)
