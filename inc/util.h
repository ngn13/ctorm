#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

typedef struct {
  char   *buf;
  int32_t size, len;
} cu_str_t;

#ifndef CTORM_EXPORT

// cu_str_t stuff
#define cu_str_empty(s) (NULL == (s)->buf || 0 == (s)->len)
#define cu_str_clear(s) bzero((s), sizeof(cu_str_t))

// char stuff
#define cu_is_digit(c) ((c) >= '0' && (c) <= '9')
#define cu_is_letter(c)                                                        \
  (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define cu_lower(c) (c | 32)

// other stuff
#define __cu_macro_to_str(x) #x
#define cu_macro_to_str(x)   __cu_macro_to_str(x)
#define cu_unused(p)         (void)p
#define cu_null_check(p, e, r)                                                 \
  if (NULL == (p)) {                                                           \
    errno = e;                                                                 \
    return r;                                                                  \
  }

int32_t cu_str_set(cu_str_t *str, char *buf);
bool    cu_str_free(cu_str_t *str);
int32_t cu_str_append(cu_str_t *str, char *buf, int32_t size);
int32_t cu_str_add(cu_str_t *str, char c);

bool     cu_streq(char *s1, char *s2);
uint64_t cu_strlen(char *str);
bool     cu_strcmpu(char *s1, char *s2, char end);
bool     cu_startswith(char *buf, char *pre);
bool     cu_endswith(char *str, char *suf);
bool     cu_contains(char *str, char c);

#endif
