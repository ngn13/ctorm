#include "../include/ctorm.h"
#include "../include/options.h"

#include "../include/log.h"
#include "../include/req.h"
#include "../include/res.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void get_time(char *res) {
  time_t    now = time(NULL);
  struct tm tm  = *localtime(&now);

  sprintf(res, "(%02d/%02d/%04d %02d:%02d)", tm.tm_mday, tm.tm_mon + 1, (tm.tm_year + 1900), tm.tm_min, tm.tm_hour);
}

void log_req(double time, req_t *req, res_t *res) {
  char tstr[25];
  get_time(tstr);

  printf(FG_MAGENTA "%s" FG_BOLD FG_MAGENTA " LOG   " FG_RESET FG_YELLO "%.0fμs" FG_RESET FG_CYAN " %d " FG_GREEN
                    "%s %s" FG_RESET,
      tstr,
      time,
      res->code,
      req_method(req),
      req->path);
  printf("\n");
}

void info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(FG_BLUE "%s" FG_RESET FG_BLUE FG_BOLD " INFO  " FG_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(FG_RED "%s" FG_RESET FG_RED FG_BOLD " ERROR " FG_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(FG_YELLO "%s" FG_RESET FG_YELLO FG_BOLD " WARN  " FG_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void _debug(const char *msg, ...) {
  if (CTORM_DEBUG == 0)
    return;

  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(FG_CYAN "%s" FG_RESET FG_CYAN FG_BOLD " DEBUG " FG_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}
