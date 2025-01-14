#include "util.h"
#include "log.h"
#include "req.h"
#include "res.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define __print_prefix(stream, prefix, color)                                                                          \
  do {                                                                                                                 \
    time_t    now   = time(NULL);                                                                                      \
    struct tm local = *localtime(&now);                                                                                \
    fprintf(stream,                                                                                                    \
        FG_GRAY "%02d/%02d/%04d %02d:%02d:%02d" FG_RESET " " color FG_BOLD prefix FG_RESET " ",                        \
        local.tm_mday,                                                                                                 \
        local.tm_mon + 1,                                                                                              \
        (local.tm_year + 1900),                                                                                        \
        local.tm_hour,                                                                                                 \
        local.tm_min,                                                                                                  \
        local.tm_sec);                                                                                                 \
  } while (0)

#define __print_log(time, time_fmt, req, res)                                                                          \
  do {                                                                                                                 \
    fprintf(stdout,                                                                                                    \
        FG_YELLO time_fmt FG_RESET FG_CYAN " %d " FG_GREEN "%" cu_to_str_macro(HTTP_METHOD_MAX) "s %s" FG_RESET "\n",  \
        time,                                                                                                          \
        res->code,                                                                                                     \
        ctorm_req_method(req),                                                                                         \
        req->encpath);                                                                                                 \
  } while (0)

void ctorm_log(ctorm_req_t *req, ctorm_res_t *res, uint64_t process_time) {
  __print_prefix(stdout, "log  ", FG_MAGENTA);

  // print as microseconds (1000000μs = 1s)
  if (process_time < 1000) {
    __print_log(process_time, "%3luμs", req, res);
    return;
  }

  // print as milliseconds (1000ms = 1s)
  if ((process_time /= 1000) < 1000) {
    __print_log(process_time, "%3lums", req, res);
    return;
  }

  // print as seconds
  __print_log(process_time /= 1000, "%3lus", req, res);
}

void ctorm_info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(stdout, "info ", FG_BLUE);
  vfprintf(stdout, msg, args);
  fprintf(stdout, "\n");

  va_end(args);
}

void ctorm_debug(const char *msg, ...) {
  if (CTORM_DEBUG == 0)
    return;

  va_list args;
  va_start(args, msg);

  __print_prefix(stdout, "debug", FG_CYAN);
  vfprintf(stdout, msg, args);
  fprintf(stdout, "\n");

  va_end(args);
}

void ctorm_fail(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(stderr, "fail ", FG_RED);
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");

  va_end(args);
}

void ctorm_warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(stderr, "warn ", FG_YELLO);
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");

  va_end(args);
}
