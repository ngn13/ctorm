#pragma once

#include "req.h"
#include "res.h"

#define FG_RED     "\x1b[31m"
#define FG_BOLD    "\x1b[1m"
#define FG_BLUE    "\x1b[34m"
#define FG_CYAN    "\x1b[36m"
#define FG_YELLO   "\x1b[33m"
#define FG_GREEN   "\x1b[32m"
#define FG_MAGENTA "\x1b[35m"
#define FG_RESET   "\x1b[0m"

#if CTORM_DEBUG
#define debug(f, ...) _debug("(" FG_BOLD "%s" FG_RESET ") " f, __func__, ##__VA_ARGS__)
#else
#define debug(...) asm("nop")
#endif

void log_req(double, req_t *, res_t *);
void info(const char *, ...);
void warn(const char *, ...);
void error(const char *, ...);
void _debug(const char *, ...);
