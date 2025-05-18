#include "util.h"
#include "http.h"

#include "log.h"
#include "req.h"
#include "res.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define log_prefix(stream, prefix, color)                                      \
  do {                                                                         \
    time_t    now   = time(NULL);                                              \
    struct tm local = *localtime(&now);                                        \
    fprintf(stream,                                                            \
        FG_GRAY "%02d/%02d/%04d %02d:%02d:%02d" FG_RESET                       \
                " " color FG_BOLD prefix FG_RESET " ",                         \
        local.tm_mday,                                                         \
        local.tm_mon + 1,                                                      \
        (local.tm_year + 1900),                                                \
        local.tm_hour,                                                         \
        local.tm_min,                                                          \
        local.tm_sec);                                                         \
  } while (0)

#define log_req(time_fmt, time, req, res)                                      \
  do {                                                                         \
    fprintf(stdout,                                                            \
        FG_YELLO time_fmt FG_RESET FG_CYAN                                     \
        " %hu " FG_GREEN                                                       \
        "%" cu_macro_to_str(CTORM_HTTP_METHOD_MAX) "s %s" FG_RESET "\n",       \
        time,                                                                  \
        res->code,                                                             \
        ctorm_req_method(req),                                                 \
        req->path);                                                            \
  } while (0)

void ctorm_log(ctorm_req_t *req, ctorm_res_t *res, uint64_t ptime) {
  log_prefix(stdout, "log  ", FG_MAGENTA);

  // print as microseconds (1000000μs = 1s)
  if (ptime < 1000) {
    log_req("%3luμs", ptime, req, res);
    return;
  }

  // print as milliseconds (1000ms = 1s)
  if ((ptime /= 1000) < 1000) {
    log_req("%3lums", ptime, req, res);
    return;
  }

  // print as seconds
  log_req("%3lus", ptime /= 1000, req, res);
}

void ctorm_info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  log_prefix(stdout, "info ", FG_BLUE);
  vfprintf(stdout, msg, args);
  fprintf(stdout, "\n");

  va_end(args);
}

void ctorm_debug(const char *msg, ...) {
#if CTORM_DEBUG
  va_list args;
  va_start(args, msg);

  log_prefix(stdout, "debug", FG_CYAN);
  vfprintf(stdout, msg, args);
  fprintf(stdout, "\n");

  va_end(args);
#endif
}

void ctorm_fail(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  log_prefix(stderr, "fail ", FG_RED);
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");

  va_end(args);
}

void ctorm_warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  log_prefix(stderr, "warn ", FG_YELLO);
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");

  va_end(args);
}
