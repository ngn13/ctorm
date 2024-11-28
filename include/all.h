/*

 * ctorm | Simple web framework for C
 * Written by ngn (https://ngn.tf) (2024)

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

#include "ctorm.h"
#include "errors.h"
#include "log.h"

#define ALL(app, path, func)     app_add(app, "", false, path, func)
#define GET(app, path, func)     app_add(app, "GET", false, path, func)
#define PUT(app, path, func)     app_add(app, "PUT", false, path, func)
#define HEAD(app, path, func)    app_add(app, "HEAD", false, path, func)
#define POST(app, path, func)    app_add(app, "POST", false, path, func)
#define DELETE(app, path, func)  app_add(app, "DELETE", false, path, func)
#define OPTIONS(app, path, func) app_add(app, "OPTIONS", false, path, func)

#define MIDDLEWARE_ALL(app, path, func)     app_add(app, "", true, path, func)
#define MIDDLEWARE_GET(app, path, func)     app_add(app, "GET", true, path, func)
#define MIDDLEWARE_PUT(app, path, func)     app_add(app, "PUT", true, path, func)
#define MIDDLEWARE_HEAD(app, path, func)    app_add(app, "HEAD", true, path, func)
#define MIDDLEWARE_POST(app, path, func)    app_add(app, "POST", true, path, func)
#define MIDDLEWARE_DELETE(app, path, func)  app_add(app, "DELETE", true, path, func)
#define MIDDLEWARE_OPTIONS(app, path, func) app_add(app, "OPTIONS", true, path, func)

#define REQ_METHOD()     req_method(req)
#define REQ_BODY_SIZE()  req_body_size(req)
#define REQ_BODY(data)   req_body(req, data)
#define REQ_GET(header)  req_get(req, header)
#define REQ_QUERY(query) req_query(req, query)
#define REQ_FORM()       req_form(req)
#define REQ_JSON()       req_json(req)

#define RES_SEND(text)         res_send(res, text, 0)
#define RES_SENDFILE(path)     res_sendfile(res, path)
#define RES_SET(header, value) res_set(res, header, value)
#define RES_DEL(header)        res_del(res, header)
#define RES_JSON(json)         res_json(res, json)
#define RES_CLEAR()            res_clear(res)
#define RES_FMT(fmt, ...)      res_fmt(res, fmt, __VA_ARGS__)
#define RES_ADD(fmt, ...)      res_add(res, fmt, __VA_ARGS__)
#define RES_REDIRECT(url)      res_redirect(res, url)
