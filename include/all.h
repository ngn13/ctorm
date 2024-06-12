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

#define ALL(path, func) app_add(app, "", false, path, func)
#define GET(path, func) app_add(app, "GET", false, path, func)
#define PUT(path, func) app_add(app, "PUT", false, path, func)
#define HEAD(path, func) app_add(app, "HEAD", false, path, func)
#define POST(path, func) app_add(app, "POST", false, path, func)
#define DELETE(path, func) app_add(app, "DELETE", false, path, func)
#define OPTIONS(path, func) app_add(app, "OPTIONS", false, path, func)

#define MIDDLEWARE_ALL(path, func) app_add(app, "", true, path, func)
#define MIDDLEWARE_GET(path, func) app_add(app, "GET", true, path, func)
#define MIDDLEWARE_PUT(path, func) app_add(app, "PUT", true, path, func)
#define MIDDLEWARE_HEAD(path, func) app_add(app, "HEAD", true, path, func)
#define MIDDLEWARE_POST(path, func) app_add(app, "POST", true, path, func)
#define MIDDLEWARE_DELETE(path, func) app_add(app, "DELETE", true, path, func)
#define MIDDLEWARE_OPTIONS(path, func) app_add(app, "OPTIONS", true, path, func)

#define APP_RUN(host) app_run(app, host)
#define APP_ALL(route) app_all(app, route)
#define APP_STATIC(path, dir) app_static(app, path, dir)

#define REQ_HEADER(header) req_header(req, header)
#define REQ_QUERY(query) req_query(req, query)
#define REQ_FORM() req_form_parse(req);
#define REQ_JSON() req_json_parse(req);

#define RES_SEND(text) res_send(res, text, 0)
#define RES_SENDFILE(path) res_sendfile(res, path)
#define RES_SET(header, value) res_set(res, header, value)
#define RES_JSON(json) res_json(res, json)
#define RES_FMT(...) res_fmt(res, __VA_ARGS__)
#define RES_ADD(...) res_fmt(res, __VA_ARGS__)
#define RES_REDIRECT(url) res_redirect(res, url)
