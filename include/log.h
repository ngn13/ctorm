#pragma once

#include "req.h"
#include "res.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLO   "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define LOG_ENABLED   log_enabled()

bool log_enabled();
void log_set(bool);
void log_req(double, req_t*, res_t*);
void info(const char*, ...);
void warn(const char*, ...);
void error(const char*, ...);
void debug(const char*, ...);
