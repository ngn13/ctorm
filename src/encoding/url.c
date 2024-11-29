#include "../../include/encoding.h"
#include "../../include/errors.h"
#include "../../include/util.h"
#include "../../include/log.h"

#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 20

enc_url_t *__enc_url_add(enc_url_t *url, char *key, char *val) {
  enc_url_t *new = malloc(sizeof(enc_url_t));
  bzero(new, sizeof(enc_url_t));

  new->pre   = url;
  new->key   = key;
  new->value = val;

  return new;
}

enc_url_t *enc_url_parse(char *data, uint64_t len) {
  if (NULL == data)
    return NULL;

  if (len == 0)
    for (char *c = data; *c != 0; c++)
      len++;

  uint64_t   key_size = 0, val_size = 0, indx = 0;
  char      *key_buf = NULL, *val_buf = NULL;
  enc_url_t *url    = NULL;
  bool       is_key = true;

  key_size = val_size = len > BUF_SIZE * 2 ? BUF_SIZE : len;

  if (NULL == (key_buf = malloc(key_size))) {
    debug("Failed to allocate a buffer for the key size");
    errno = AllocFailed;
    return NULL;
  }

  if (NULL == (val_buf = malloc(val_size))) {
    debug("Failed to allocate a buffer for the value size");
    errno = AllocFailed;
    return NULL;
  }

  for (; len > 0; len--, data++) {
    if (*data == '=' && is_key) {
      is_key = false;
      indx   = 0;
      continue;
    }

    if (!is_key && *data == '&')
      goto url_add;

    if (is_key) {
      if (indx + 1 >= key_size)
        key_buf = realloc(key_buf, key_size *= 2);
      key_buf[indx++] = *data;
      key_buf[indx]   = 0;
    }

    else {
      if (indx + 1 >= val_size)
        val_buf = realloc(val_buf, val_size *= 2);
      val_buf[indx++] = *data;
      val_buf[indx]   = 0;
    }

    if (len != 1)
      continue;

  url_add:
    is_key = true;
    indx   = 0;

    urldecode(key_buf);
    urldecode(val_buf);

    url = __enc_url_add(url, key_buf, val_buf);
  }

  return url;
}

char *enc_url_get(enc_url_t *url, char *key) {
  while (NULL != url) {
    if (strcmp(url->key, key) == 0)
      return url->value;
    url = url->pre;
  }

  return NULL;
}

void enc_url_free(enc_url_t *url) {
  enc_url_t *pre = NULL;

  while (NULL != url) {
    pre = url->pre;
    free(url->value);
    free(url->key);
    free(url);
    url = pre;
  }
}
