#include "log.h"
#include "req.h"
#include "res.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define __print_prefix(prefix, color)                                                                                  \
  do {                                                                                                                 \
    time_t    now   = time(NULL);                                                                                      \
    struct tm local = *localtime(&now);                                                                                \
    printf(color "(%02d/%02d/%04d %02d:%02d)" FG_RESET color FG_BOLD prefix FG_RESET,                                  \
        local.tm_mday,                                                                                                 \
        local.tm_mon + 1,                                                                                              \
        (local.tm_year + 1900),                                                                                        \
        local.tm_min,                                                                                                  \
        local.tm_hour);                                                                                                \
  } while (0)

void ctorm_log(ctorm_req_t *req, ctorm_res_t *res, uint64_t process_time) {
  __print_prefix(" LOG   ", FG_MAGENTA);
  printf(FG_YELLO "%5luμs" FG_RESET FG_CYAN " %d " FG_GREEN "%s %s" FG_RESET,
      process_time,
      res->code,
      ctorm_req_method(req),
      req->path);
  printf("\n");
}

void info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(" INFO  ", FG_BLUE);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(" ERROR ", FG_RED);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  __print_prefix(" WARN  ", FG_YELLO);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void _debug(const char *msg, ...) {
  if (CTORM_DEBUG == 0)
    return;

  va_list args;
  va_start(args, msg);

  __print_prefix(" DEBUG ", FG_CYAN);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}
