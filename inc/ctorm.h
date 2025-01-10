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

#define CTORM_EXPORT

#include "app.h"
#include "req.h"
#include "res.h"
#include "log.h"
#include "errors.h"

#define ALL(app, path, func)     ctorm_app_add(app, NULL, false, path, func)
#define GET(app, path, func)     ctorm_app_add(app, "GET", false, path, func)
#define PUT(app, path, func)     ctorm_app_add(app, "PUT", false, path, func)
#define HEAD(app, path, func)    ctorm_app_add(app, "HEAD", false, path, func)
#define POST(app, path, func)    ctorm_app_add(app, "POST", false, path, func)
#define DELETE(app, path, func)  ctorm_app_add(app, "DELETE", false, path, func)
#define OPTIONS(app, path, func) ctorm_app_add(app, "OPTIONS", false, path, func)

#define MIDDLEWARE_ALL(app, path, func)     ctorm_app_add(app, NULL, true, path, func)
#define MIDDLEWARE_GET(app, path, func)     ctorm_app_add(app, "GET", true, path, func)
#define MIDDLEWARE_PUT(app, path, func)     ctorm_app_add(app, "PUT", true, path, func)
#define MIDDLEWARE_HEAD(app, path, func)    ctorm_app_add(app, "HEAD", true, path, func)
#define MIDDLEWARE_POST(app, path, func)    ctorm_app_add(app, "POST", true, path, func)
#define MIDDLEWARE_DELETE(app, path, func)  ctorm_app_add(app, "DELETE", true, path, func)
#define MIDDLEWARE_OPTIONS(app, path, func) ctorm_app_add(app, "OPTIONS", true, path, func)

#define REQ_METHOD()           ctorm_req_method(req)
#define REQ_BODY_SIZE()        ctorm_req_body_size(req)
#define REQ_BODY(buffer, size) ctorm_req_body(req, buffer, size)
#define REQ_GET(header)        ctorm_req_get(req, header)
#define REQ_QUERY(query)       ctorm_req_query(req, query)
#define REQ_FORM()             ctorm_req_form(req)
#define REQ_JSON()             ctorm_req_json(req)
#define REQ_CANCEL()           (req->cancel = true)

#define RES_CODE(_code)        (res->code = _code)
#define RES_SEND(text)         ctorm_res_send(res, text, 0)
#define RES_SENDFILE(path)     ctorm_res_sendfile(res, path)
#define RES_SET(header, value) ctorm_res_set(res, header, value)
#define RES_DEL(header)        ctorm_res_del(res, header)
#define RES_JSON(json)         ctorm_res_json(res, json)
#define RES_CLEAR()            ctorm_res_clear(res)
#define RES_FMT(fmt, ...)      ctorm_res_fmt(res, fmt, __VA_ARGS__)
#define RES_ADD(fmt, ...)      ctorm_res_add(res, fmt, __VA_ARGS__)
#define RES_REDIRECT(url)      ctorm_res_redirect(res, url)
