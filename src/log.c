#include "util.h"
#include "http.h"

#include "log.h"
#include "req.h"
#include "res.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#ifdef __i386__

#define LOG_MICROSECOND "%3lluμs"
#define LOG_MILLISECOND "%3llums"
#define LOG_SECOND      "%3llus "

#else

#define LOG_MICROSECOND "%3luμs"
#define LOG_MILLISECOND "%3lums"
#define LOG_SECOND      "%3lus "

#endif

int _ctorm_log_prefix(FILE *stream, const char *color, const char *prefix) {
  time_t     now   = time(NULL);
  struct tm *local = localtime(&now);

  return fprintf(stream,
      FG_GRAY "%02d/%02d/%04d %02d:%02d:%02d" FG_RESET " %s%5s" FG_BOLD FG_RESET
              " ",
      local->tm_mday,
      local->tm_mon + 1,
      (local->tm_year + 1900),
      local->tm_hour,
      local->tm_min,
      local->tm_sec,
      color,
      prefix);
}

int _ctorm_log_req(ctorm_req_t *req, ctorm_res_t *res) {
  return fprintf(stdout,
      FG_RESET FG_CYAN " %hu " FG_GREEN "%" cu_macro_to_str(
          CTORM_HTTP_METHOD_MAX) "s %s" FG_RESET "\n",
      res->code,
      ctorm_req_method(req),
      req->path);
}

int ctorm_log(ctorm_req_t *req, ctorm_res_t *res, uint64_t time) {
  int size = _ctorm_log_prefix(stdout, FG_MAGENTA, "log");

  // check if the time is valid or not
  if (time == 0)
    size += fprintf(stdout, FG_YELLO "???ms" FG_RESET);

  // print as microseconds (1000000μs = 1s)
  else if (time < 1000)
    size += fprintf(stdout, FG_YELLO LOG_MICROSECOND FG_RESET, time);

  // print as milliseconds (1000ms = 1s)
  else if ((time /= 1000) < 1000)
    size += fprintf(stdout, FG_YELLO LOG_MILLISECOND FG_RESET, time);

  // print as seconds
  else
    size += fprintf(stdout, FG_YELLO LOG_SECOND FG_RESET, time / 1000);

  return size + _ctorm_log_req(req, res);
}

int ctorm_info(const char *msg, ...) {
  va_list args;
  int     size = 0;

  va_start(args, msg);

  size += _ctorm_log_prefix(stdout, FG_BLUE, "info");
  size += vfprintf(stdout, msg, args);
  size += fprintf(stdout, "\n");

  va_end(args);
  return size;
}

int ctorm_debug(const char *msg, ...) {
#if CTORM_DEBUG
  va_list args;
  int     size = 0;

  va_start(args, msg);

  size += _ctorm_log_prefix(stdout, FG_CYAN, "debug");
  size += vfprintf(stdout, msg, args);
  size += fprintf(stdout, "\n");

  va_end(args);
  return size;
#else
  cu_unused(msg);
  return -1;
#endif
}

int ctorm_fail(const char *msg, ...) {
  va_list args;
  int     size = 0;

  va_start(args, msg);

  size += _ctorm_log_prefix(stderr, FG_RED, "fail");
  size += vfprintf(stderr, msg, args);
  size += fprintf(stderr, "\n");

  va_end(args);
  return size;
}

int ctorm_warn(const char *msg, ...) {
  va_list args;
  int     size = 0;

  va_start(args, msg);

  size += _ctorm_log_prefix(stderr, FG_YELLO, "warn");
  size += vfprintf(stderr, msg, args);
  size += fprintf(stderr, "\n");

  va_end(args);
  return size;
}
