#include "util.h"

#include <sys/stat.h>
#include <stdbool.h>

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <fcntl.h>
#include <stdio.h>

bool cu_endswith(char *str, char *suf) {
  uint64_t str_len = cu_strlen(str);
  uint64_t suf_len = cu_strlen(suf);

  if (suf_len > str_len)
    return false;

  return cu_streq(str + (str_len - suf_len), suf);
}

char *cu_join(char *p1, char *p2) {
  char *fp = NULL;

  if ((fp = malloc(cu_strlen(p1) + cu_strlen(p2) + 2)) == NULL)
    return NULL;

  sprintf(fp, "%s/%s", p1, p2);
  return fp;
}

bool cu_contains(char *str, char c) {
  for (; *str != 0; str++)
    if (*str == c)
      return true;
  return false;
}

int64_t cu_url_decode(char *str) {
  if (NULL == str)
    return -1;

  char    *cur = NULL, *step = NULL;
  uint64_t len   = 0;
  uint8_t  value = 0;

  for (cur = step = str; *cur != 0; cur++, step++, len++) {
    if (*cur == '+') {
      *step = ' ';
      continue;
    }

    if (*cur == '%') {
      sscanf(++cur, "%02hhx", &value);
      *step = value;
      cur++;
      continue;
    }

    *step = *cur;
  }

  *step = 0;
  return len;
}
