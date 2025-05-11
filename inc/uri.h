#pragma once
#include "enc/query.h"

typedef struct {
  char          *scheme, *userinfo, *host, *fragment;
  ctorm_query_t *query;
  uint16_t       port;
} ctorm_uri_t;

#ifndef CTORM_EXPORT
#include <stdbool.h>

bool ctorm_uri_parse_path(ctorm_uri_t *uri, char *path);
bool ctorm_uri_parse_auth(ctorm_uri_t *uri, char *auth);
bool ctorm_uri_parse(ctorm_uri_t *uri, char *encoded);

#endif
