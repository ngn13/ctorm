#include "enc/query.h"
#include "enc/percent.h"

#include "pair.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

ctorm_query_t *ctorm_query_parse(char *data, uint64_t size) {
  if (NULL == data)
    return NULL;

  if (size == 0)
    size = cu_strlen(data);

  ctorm_query_t *query  = NULL;
  bool           is_key = true, ignore_val = false;
  cu_str_t       buf;

  // initialize the buffer
  cu_str_init(buf);

  for (; size > 0; size--, data++) {
    /*

     * key=value
     *    ^
     * at this position we should add a new pair, as we have read the entire key
     * into the buffer

     * how ever it possible that the data just starts with '=' so we check indx
     * to make sure we don't add a pair with an empty key

   */
    if (*data == '=' && is_key) {
      // if the value is empty, ignore it
      if (!(ignore_val = cu_str_empty(buf))) {
        ctorm_percent_decode(cu_str(buf), cu_str_len(buf));
        ctorm_pair_add(&query, cu_str(buf), NULL); // create a new pair
        cu_str_init(buf); // reset the buffer (without freeing)
      }

      is_key = false; // we are no longer reading the key
      continue;
    }

    /*

     * key=value&other_key=other_value
     *          ^
     * this is our current position if the following condition succeeds in that
     * case, we have the query value in the buffer, we should add it to the last
     * pair

    */
    if (!is_key && *data == '&')
      goto url_value_add;

    // otherwise just keep appending to the current buffer
    cu_str_add(&buf, *data);

    /*

     * key=value\0
     *         ^
     * this is our position if the following condition fails, in that case we
     * read the entire value and we should add it to the last pair

    */
    if (size != 1)
      continue;

  url_value_add:
    if (!ignore_val) {
      ctorm_percent_decode(cu_str(buf), cu_str_len(buf));
      query->value = cu_str(buf);
      cu_str_init(buf);
    }

    is_key = true;
  }

  cu_str_free(&buf);
  return query;
}

char *ctorm_query_get(ctorm_query_t *query, char *name) {
  if ((query = ctorm_pair_find(query, name)) == NULL)
    return NULL;
  return query->value;
}

void ctorm_query_free(ctorm_query_t *query) {
  ctorm_pair_next(query, cur) {
    free(cur->key);
    free(cur->value);
  }

  ctorm_pair_free(query);
}
