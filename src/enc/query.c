#include "enc/query.h"
#include "enc/percent.h"

#include "error.h"
#include "pair.h"
#include "util.h"
#include "log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QUERY_KEY_MAX   (256)
#define QUERY_VALUE_MAX (1024)

ctorm_query_t *ctorm_query_parse(char *data, uint64_t size) {
  if (NULL == data)
    return NULL;

  if (size == 0)
    size = cu_strlen(data);

  ctorm_query_t *query  = NULL;
  bool           is_key = true;
  cu_str_t       cur;

  // clear the string structure
  cu_str_clear(&cur);

  for (; size > 0; size--, data++) {
    // check the size of the current string buffer
    if (is_key && cur.len > QUERY_KEY_MAX) {
      errno = CTORM_ERR_QUERY_KEY_TOO_LARGE;
      goto fail;
    }

    if (!is_key && cur.len > QUERY_VALUE_MAX) {
      errno = CTORM_ERR_QUERY_VALUE_TOO_LARGE;
      goto fail;
    }

    /*

     * key=value
     *    ^
     * at this position we should add a new pair, as we have read the entire key
     * into the buffer

     * however it is possible that the data just starts with '=' so we also
     * check if the buffer is empty to make sure we don't add a pair with an
     * empty key

   */
    if (*data == '=' && is_key && !cu_str_empty(&cur)) {
      // percent decode the current string buffer and create a new pair
      cur.len = ctorm_percent_decode(cur.buf, cur.len);
      ctorm_pair_add(&query, cur.buf, NULL);

      cu_str_clear(&cur);
      is_key = false;
      continue;
    }

    if ((*data != '&' && size != 1) || is_key) {
      cu_str_add(&cur, *data);
      continue;
    }

    /*

     * key=value\0
     *         ^

     * at this position we should the 'e' before moving on and adding the value
     * to the last pair

    */
    if (*data != '&' && !is_key)
      cu_str_add(&cur, *data);

    /*

     * key=value&other_key=other_value
     *          ^

     * at this point we have the entire value in the string buffer, we can just
     * add the value to the last pair, associating it with the last key we added

    */
    if (cu_str_empty(&cur))
      continue;

    // percent decode the query value
    cur.len = ctorm_percent_decode(cur.buf, cur.len);

    // store it in the last pair
    if (NULL != query)
      query->value = cur.buf;

    cu_str_clear(&cur);
    is_key = true;
  }

  cu_str_free(&cur);

  if (NULL == query)
    errno = CTORM_ERR_EMPTY_QUERY;

  return query;

fail:
  cu_str_free(&cur);
  ctorm_query_free(query);
  return NULL;
}

char *ctorm_query_get(ctorm_query_t *query, char *name) {
  if ((query = ctorm_pair_find(query, name)) == NULL)
    return NULL;
  return query->value;
}

void ctorm_query_free(ctorm_query_t *query) {
  if (NULL == query)
    return;

  ctorm_pair_next(query, cur) {
    free(cur->key);
    free(cur->value);
  }

  ctorm_pair_free(query);
}
