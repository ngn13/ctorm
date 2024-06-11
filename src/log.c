#include "../include/ctorm.h"
#include "../include/req.h"
#include "../include/res.h"
#include "../include/log.h"

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

  printf(COLOR_MAGENTA "%s" COLOR_BOLD COLOR_MAGENTA " [ log ] " COLOR_RESET COLOR_CYAN "(%.0fμs)" COLOR_RESET COLOR_GREEN
                                  " %s %s" COLOR_CYAN " => " COLOR_RESET "%d",
      tstr,
      time,
      req_method(req),
      req->path,
      res->code);
  printf("\n");
}

void info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_BLUE "%s" COLOR_RESET COLOR_BLUE COLOR_BOLD " [info ] " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_RED "%s" COLOR_RESET COLOR_RED COLOR_BOLD " [error] " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_YELLO "%s" COLOR_RESET COLOR_YELLO COLOR_BOLD " [warn ] " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void debug(const char *msg, ...) {
  if (DEBUG == 0)
    return;

  va_list args;
  va_start(args, msg);

  char tstr[25];
  get_time(tstr);

  printf(COLOR_CYAN "%s" COLOR_RESET COLOR_CYAN COLOR_BOLD  " [debug] " COLOR_RESET, tstr);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}
