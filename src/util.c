#include "util.h"
#include <string.h>
#include <stdlib.h>

#define _cu_str_alloc(s, l, e)                                                 \
  if (l >= (s)->size - (s)->len) {                                             \
    if (NULL == (s)->buf)                                                      \
      (s)->buf = malloc((s)->size += e);                                       \
    else                                                                       \
      (s)->buf = realloc((s)->buf, (s)->size += e);                            \
  }

int32_t cu_str_set(cu_str_t *str, char *buf) {
  if (NULL == str || NULL == buf)
    return -1;

  str->buf        = buf;
  return str->len = str->size = cu_strlen(buf);
}

bool cu_str_clear(cu_str_t *str) {
  if (NULL == str)
    return false;

  bzero(str->buf, str->size);
  str->len = 0;
  return true;
}

bool cu_str_free(cu_str_t *str) {
  if (NULL == str)
    return false;

  free(str->buf);
  str->len = str->size = (uint64_t)(str->buf = NULL);
  return true;
}

int32_t cu_str_append(cu_str_t *str, char *buf, int32_t size) {
  if (NULL == str || NULL == buf)
    return -1;

  // if no size is specified, get string length of the buffer
  if (size == 0)
    size = cu_strlen(buf);

  // allocate if the str buffer is too small
  _cu_str_alloc(str, size, 32);

  // copy from buffer to the str buffer
  memcpy(str->buf, buf, size);
  str->len += size;
  str->buf[str->len] = 0;

  return str->len;
}

int32_t cu_str_add(cu_str_t *str, char c) {
  if (NULL == str)
    return -1;

  // allocate if the str buffer is too small
  _cu_str_alloc(str, 1, 16);

  // add char to the str buffer
  str->buf[str->len++] = c;
  str->buf[str->len]   = 0;

  return str->len;
}

bool cu_endswith(char *str, char *suf) {
  uint64_t str_len = cu_strlen(str);
  uint64_t suf_len = cu_strlen(suf);

  if (suf_len > str_len)
    return false;

  return cu_streq(str + (str_len - suf_len), suf);
}
