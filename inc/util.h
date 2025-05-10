#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char    *buf;
  uint32_t len;
} cu_str_t;

#ifndef CTORM_EXPORT

// cu_str_t stuff
#define cu_str(s)          (NULL == (s).buf ? "(empty)" : (s).buf)
#define cu_str_is_empty(s) (NULL == (s).buf || 0 == (s).len)
#define cu_str_set(s, p)                                                       \
  do {                                                                         \
    if (NULL != p) {                                                           \
      (s).buf = p;                                                             \
      (s).len = cu_strlen(p);                                                  \
    }                                                                          \
  } while (0)

// char stuff
#define cu_is_digit(c) ((c) >= '0' && (c) <= '9')
#define cu_is_letter(c)                                                        \
  (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define cu_lower(c) (c | 32)

// other stuff
#define __cu_macro_to_str(x) #x
#define cu_macro_to_str(x)   __cu_macro_to_str(x)
#define cu_unused(p)         (void)p

bool     cu_streq(const char *s1, const char *s2); // compare 2 strings
uint64_t cu_strlen(char *str);                     // get length of a string
bool     cu_startswith(
        char *str, char *pre); // check if a string starts with a given prefix
bool cu_endswith(
    char *str, char *suf);         // check if a string ends with a given suffix
char *cu_join(char *p1, char *p2); // join two paths together
bool  cu_contains(char *str, char c); // check if a string contains a char
bool  cu_strcmp_until(
     char *s1, char *s2, char c); // compare 2 strings until a char is reached
int64_t cu_url_decode(char *str); // URL decode a given string

#endif
