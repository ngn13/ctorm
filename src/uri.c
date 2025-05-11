#include "uri.h"
#include "error.h"
#include "util.h"

#include <stdlib.h>
#include <stdint.h>

bool ctorm_uri_parse_path(ctorm_uri_t *uri, char *path) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, false);
  cu_null_check(path, CTORM_ERR_BAD_PATH_PTR, false);

  // TODO: implement
  return false;
}

bool ctorm_uri_parse_auth(ctorm_uri_t *uri, char *auth) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, false);
  cu_null_check(auth, CTORM_ERR_BAD_AUTH_PTR, false);

  // TODO: implement
  return false;
}

bool ctorm_uri_parse(ctorm_uri_t *uri, char *encoded) {
  if (NULL == uri || NULL == encoded) {
    errno = CTORM_ERR_BAD_URI_PTR;
    return false;
  }

  // TODO: implement
  return false;
}
