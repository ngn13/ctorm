#include "encoding.h"
#include "error.h"
#include "pair.h"
#include "util.h"
#include "log.h"

#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

#define BUF_SIZE 16

ctorm_url_t *ctorm_url_parse(char *data, uint64_t len) {
  if (NULL == data)
    return NULL;

  if (len == 0)
    for (char *c = data; *c != 0; c++)
      len++;

  bool         is_key = true, ignore_val = false;
  uint64_t     buf_size = 0, indx = 0;
  ctorm_url_t *url = NULL;
  char        *buf = NULL;

  for (; len > 0; len--, data++) {
    // allocate a new buffer if required
    if (NULL == buf) {
      buf_size = BUF_SIZE > len + 1 ? len + 1 : BUF_SIZE;
      indx     = 0;

      if (NULL == (buf = malloc(buf_size))) {
        debug("failed to allocate a buffer for the key size");
        errno = CTORM_ERR_ALLOC_FAIL;
        return NULL;
      }
    }

    /*

     * key=value
     *    ^
     * at this position we should add a new pair, as we have
     * read the entire key into the buf

     * how ever it possible that the data just starts with '='
     * so we check indx to make sure we don't add a pair with an empty key

   */
    if (*data == '=' && is_key) {
      if (!(ignore_val = (indx == 0))) {
        cu_url_decode(buf);
        ctorm_pair_add(&url, buf, NULL); // not empty? create a new pair
        buf = NULL; // reset the buffer (it's now used by the pair)
      }

      is_key = false; // we are no longer reading the key
      continue;
    }

    /*

     * key=value&other_key=other_value
     *          ^
     * this is our current position if the following condition succeeds
     * in that case, we have the query value in the buf, we should add it
     * to the last pair

    */
    if (!is_key && *data == '&')
      goto url_value_add;

    if (indx + 1 >= buf_size)
      buf = realloc(buf, buf_size *= 2);
    buf[indx++] = *data;
    buf[indx]   = 0;

    /*

     * key=value\0
     *         ^
     * this is our position if the following condition fails, in that case
     * we read the entire value and we should add it to the last pair

    */
    if (len != 1)
      continue;

  url_value_add:
    if (!ignore_val) {
      cu_url_decode(buf);
      url->value = buf;
      buf        = NULL;
    }

    is_key = true;
  }

  free(buf);
  return url;
}

char *ctorm_url_get(ctorm_url_t *url, char *name) {
  if ((url = ctorm_pair_find(url, name)) == NULL)
    return NULL;
  return url->value;
}

void ctorm_url_free(ctorm_url_t *url) {
  ctorm_pair_next(url, cur) {
    free(cur->key);
    free(cur->value);
  }

  ctorm_pair_free(url);
}
