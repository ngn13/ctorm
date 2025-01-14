#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char    *str;
  uint64_t len;
} cu_str_t;

#ifndef CTORM_EXPORT

#define cu_str_is_empty(p) (NULL == (p).str)
#define cu_str_set(p, s)                                                                                               \
  do {                                                                                                                 \
    if (NULL != s) {                                                                                                   \
      (p).str = s;                                                                                                     \
      (p).len = cu_strlen(s);                                                                                          \
    }                                                                                                                  \
  } while (0)

#define cu_is_digit(c)  ((c) >= '0' && (c) <= '9')
#define cu_is_letter(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define cu_lower(c)     (c | 32)

#define __cu_to_str_macro(x) #x
#define cu_to_str_macro(x)   __cu_to_str_macro(x)

#define cu_truncate(buf, size, indx, ch)                                                                               \
  if (size >= indx && buf[size - indx] == ch)                                                                          \
  buf[size - indx] = 0

bool     cu_streq(const char *s1, const char *s2);    // compare 2 strings
uint64_t cu_strlen(char *str);                        // get length of a string
bool     cu_startswith(char *str, char *pre);         // check if a string starts with a given prefix
bool     cu_endswith(char *str, char *suf);           // check if a string ends with a given suffix
char    *cu_join(char *p1, char *p2);                 // join two paths together
bool     cu_contains(char *str, char c);              // check if a string contains a char
bool     cu_strcmp_until(char *s1, char *s2, char c); // compare 2 strings until a char is reached
void     cu_url_decode(char *str);                    // URL decode a given string

#endif
