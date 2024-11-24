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

  printf(COLOR_MAGENTA "%s" COLOR_BOLD COLOR_MAGENTA " LOG   " COLOR_RESET COLOR_YELLO "%.0fμs" COLOR_RESET COLOR_CYAN
                       " %d " COLOR_GREEN "%s %s" COLOR_RESET,
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

  printf(COLOR_BLUE "%s" COLOR_RESET COLOR_BLUE COLOR_BOLD " INFO  " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_RED "%s" COLOR_RESET COLOR_RED COLOR_BOLD " ERROR " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_YELLO "%s" COLOR_RESET COLOR_YELLO COLOR_BOLD " WARN  " COLOR_RESET, tstr);
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

  printf(COLOR_CYAN "%s" COLOR_RESET COLOR_CYAN COLOR_BOLD " DEBUG " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}
