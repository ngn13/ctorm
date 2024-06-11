#pragma once

#include "ctorm.h"

#define APP_RUN(host) app_run(app, host)
#define APP_ALL(route) app_all(app, route)
#define APP_STATIC(path, dir) app_static(app, path, dir)

#define REQ_HEADER(header) req_header(req, header)
#define REQ_QUERY(query) req_query(req, query)

#define RES_SEND(text) res_send(res, text, 0)
#define RES_SENDFILE(path) res_sendfile(res, path)
#define RES_SET(header, value) res_set(res, header, value)
