#pragma once

#include "req.h"
#include "res.h"

#ifndef CTORM_EXPORT

#define FG_RED     "\x1b[31m"
#define FG_BOLD    "\x1b[1m"
#define FG_BLUE    "\x1b[34m"
#define FG_CYAN    "\x1b[36m"
#define FG_YELLO   "\x1b[33m"
#define FG_GREEN   "\x1b[32m"
#define FG_MAGENTA "\x1b[35m"
#define FG_RESET   "\x1b[0m"

#define log(req, res, pt, ...) ctorm_log(req, res, pt)
#define info(fmt, ...)         ctorm_info(fmt, ##__VA_ARGS__)
#define warn(fmt, ...)         ctorm_warn(fmt, ##__VA_ARGS__)
#define error(fmt, ...)        ctorm_error(fmt, ##__VA_ARGS__)

#if CTORM_DEBUG
#define debug(fmt, ...) ctorm_debug("(" FG_BOLD "%s" FG_RESET ") " fmt, __func__, ##__VA_ARGS__)
#else
#define debug(...) asm("nop")
#endif

#endif

void ctorm_log(ctorm_req_t *req, ctorm_res_t *res, uint64_t process_time);
void ctorm_info(const char *fmt, ...);
void ctorm_warn(const char *fmt, ...);
void ctorm_error(const char *fmt, ...);
void ctorm_debug(const char *fmt, ...);
