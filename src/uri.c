#include "encoding.h"
#include "error.h"
#include "util.h"
#include "uri.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// all the references can be found in RFC 3986 (URI: Generic Syntax)

#define URI_SCHEME_MAX   (256)
#define URI_USERINFO_MAX (256)
#define URI_HOST_MAX     (255)
#define URI_PORT_MAX     (5)
#define URI_PATH_MAX     (2048)
#define URI_QUERY_MAX    (1024)
#define URI_FRAGMENT_MAX (64)

#define URI_PORT_NUM_MAX (UINT16_MAX)
#define URI_PORT_NUM_MIN (1)

#define URI_ABNF_UNRESERVED "-._~" // / ALPHA / DIGIT
#define URI_ABNF_SUB_DELIMS "!$&'()*+,;="
#define URI_ABNF_PCHAR      URI_ABNF_UNRESERVED URI_ABNF_SUB_DELIMS ":@%"

// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
#define uri_check_scheme(c)                                                    \
  (cu_is_letter((c)) || cu_is_digit((c)) || cu_contains("+-.", (c)))

// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
#define uri_check_userinfo(c)                                                  \
  (cu_is_letter((c)) || cu_is_digit((c)) ||                                    \
      cu_contains(URI_ABNF_UNRESERVED URI_ABNF_SUB_DELIMS "%:", (c)))

// host = IP-literal / IPv4address / reg-name
#define uri_check_host(c)                                                      \
  (cu_is_letter((c)) || cu_is_digit((c)) ||                                    \
      cu_contains(URI_ABNF_UNRESERVED URI_ABNF_SUB_DELIMS "[:]%", (c)))

#define uri_check_path(c)                                                      \
  (cu_is_letter((c)) || cu_is_digit((c)) ||                                    \
      cu_contains("/" URI_ABNF_UNRESERVED URI_ABNF_SUB_DELIMS "%:@", (c)))

// query = *( pchar / "/" / "?" )
#define uri_check_query(c)                                                     \
  (cu_is_letter((c)) || cu_is_digit((c)) ||                                    \
      cu_contains(URI_ABNF_PCHAR "/?", (c)))

void ctorm_uri_free(ctorm_uri_t *uri) {
  // free all the components
  free(uri->scheme);
  free(uri->userinfo);
  free(uri->host);
  free(uri->path);
  ctorm_query_free(uri->query);
  free(uri->fragment);

  // clear the ctorm_uri_t structure
  memset(uri, 0, sizeof(*uri));
}

bool ctorm_uri_parse(ctorm_uri_t *uri, char *str) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, false);
  cu_null_check(uri, CTORM_ERR_BAD_DATA_PTR, false);

  // the scheme should start with a letter (3.1. Scheme)
  if (!cu_is_letter(*str)) {
    errno = CTORM_ERR_BAD_SCHEME;
    return false;
  }

  cu_str_t scheme;
  cu_str_clear(&scheme);

  // read the scheme into the string buffer
  for (; *str != 0 && *str != ':'; str++) {
    // check the current char
    if (!uri_check_scheme(*str)) {
      errno = CTORM_ERR_BAD_SCHEME;
      goto fail;
    }

    // check the scheme length
    if (scheme.len + 1 > URI_SCHEME_MAX) {
      errno = CTORM_ERR_SCHEME_TOO_LARGE;
      goto fail;
    }

    // add char to the scheme
    cu_str_add(&scheme, *str);
  }

  // scheme is follow by ":" and then the hier-part (3. Syntax Components)
  if (*str != ':') {
    errno = CTORM_ERR_BAD_URI_FORMAT;
    goto fail;
  }

  // save the scheme and clear the string buffer
  uri->scheme = scheme.buf;
  cu_str_clear(&scheme);

  // move to hier part
  str++;

  if (cu_startswith(str, "//") &&
      NULL == (str = ctorm_uri_parse_auth(uri, str + 2)))
    goto fail; // errno set by ctorm_uri_parse_auth()

  if (NULL == ctorm_uri_parse_path(uri, str))
    goto fail; // errno set by ctorm_uri_parse_path()

  return true;
fail:
  ctorm_uri_free(uri);
  cu_str_free(&scheme);
  return false;
}

char *ctorm_uri_parse_auth(ctorm_uri_t *uri, char *auth) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, NULL);
  cu_null_check(auth, CTORM_ERR_BAD_AUTHORITY_PTR, NULL);

  bool     has_userinfo = false;
  char    *start        = auth;
  cu_str_t userinfo;

  // check if authority contains userinfo
  for (; *auth != 0 && *auth != '/'; auth++)
    if ((has_userinfo = *auth == '@'))
      break;

  cu_str_clear(&userinfo);
  auth = start;

  if (has_userinfo) {
    for (; *auth != '@'; auth++) {
      // check the current char
      if (!uri_check_userinfo(*auth)) {
        errno = CTORM_ERR_BAD_USERINFO;
        goto fail;
      }

      // check the length
      if (userinfo.len + 1 > URI_USERINFO_MAX) {
        errno = CTORM_ERR_USERINFO_TOO_LARGE;
        goto fail;
      }

      // add char to userinfo
      cu_str_add(&userinfo, *auth);
    }

    // percent decode the userinfo data and save it
    userinfo.len  = ctorm_percent_decode(userinfo.buf, userinfo.len);
    uri->userinfo = userinfo.buf;
    cu_str_clear(&userinfo);
  }

  // read the host component
  if (NULL == (auth = ctorm_uri_parse_host(uri, auth)))
    goto fail; // errno set by ctorm_uri_parse_host()

  return auth;
fail:
  cu_str_free(&userinfo);
  free(uri->userinfo);
  return uri->userinfo = NULL;
}

char *ctorm_uri_parse_host(ctorm_uri_t *uri, char *addr) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, NULL);
  cu_null_check(addr, CTORM_ERR_BAD_HOST_PTR, NULL);

  cu_str_t host;
  bool     bracket = false;

  cu_str_clear(&host);

  for (; *addr != 0 && *addr != '/'; addr++) {
    // if we are not inside a bracket, ":" means we reached a port number
    if (!bracket && *addr == ':')
      break;

    if (!uri_check_host(*addr)) {
      errno = CTORM_ERR_BAD_HOST;
      goto fail;
    }

    if (host.len + 1 > URI_HOST_MAX) {
      errno = CTORM_ERR_HOST_TOO_LARGE;
      goto fail;
    }

    // check if we are inside/outside a bracket
    switch (*addr) {
    case '[':
      bracket = true;
      break;

    case ']':
      bracket = false;
      break;
    }

    // add char to the host string
    cu_str_add(&host, *addr);
  }

  // percent decode and save the host string buffer
  host.len  = ctorm_percent_decode(host.buf, host.len);
  uri->host = host.buf;
  cu_str_clear(&host);

  if (*addr != ':')
    return addr;

  char port[URI_PORT_MAX + 1];
  int  indx = 0, num = 0;

  memset(port, 0, sizeof(port));

  for (addr++; *addr != 0 && *addr != '/'; addr++) {
    if (indx >= URI_PORT_MAX) {
      errno = CTORM_ERR_PORT_TOO_LARGE;
      goto fail;
    }

    port[indx++] = *addr;
  }

  if ((num = atoi(port)) > URI_PORT_NUM_MAX || num < URI_PORT_NUM_MIN) {
    errno = CTORM_ERR_BAD_PORT;
    goto fail;
  }

  uri->port = (uint16_t)num;
  return addr;
fail:
  cu_str_free(&host);
  uri->port = 0;
  free(uri->host);
  return uri->host = NULL;
}

char *ctorm_uri_parse_path(ctorm_uri_t *uri, char *path) {
  cu_null_check(uri, CTORM_ERR_BAD_URI_PTR, NULL);
  cu_null_check(path, CTORM_ERR_BAD_PATH_PTR, NULL);

  // path-empty
  if (*path == 0)
    return path;

  cu_str_t str;
  cu_str_clear(&str);

  if (*path != '/')
    cu_str_add(&str, '/');

  for (; *path != 0 && *path != '?' && *path != '#'; path++) {
    // check the path char
    if (!uri_check_path(*path)) {
      errno = CTORM_ERR_BAD_PATH;
      goto fail;
    }

    // check the length of the path
    if (str.len > URI_PATH_MAX) {
      errno = CTORM_ERR_PATH_TOO_LARGE;
      goto fail;
    }

    // add char to the path
    cu_str_add(&str, *path);
  }

  // percent decode the path and save it
  str.len   = ctorm_percent_decode(str.buf, str.len);
  uri->path = str.buf;
  cu_str_clear(&str);

  if (*path == 0)
    return path;

  if (*path == '?' && *(++path) != 0) {
    for (; *path != 0 && *path != '#'; path++) {
      // check the query char
      if (!uri_check_query(*path)) {
        errno = CTORM_ERR_BAD_QUERY;
        goto fail;
      }

      // check the length of the query
      if (str.len > URI_QUERY_MAX) {
        errno = CTORM_ERR_QUERY_TOO_LARGE;
        goto fail;
      }

      // add char to the query
      cu_str_add(&str, *path);
    }
  }

  uri->query = ctorm_query_parse(str.buf, str.len);
  cu_str_clear(&str);

  if (*path == 0)
    return path;

  for (; *path != 0; path++) {
    // check the query char
    if (!uri_check_query(*path)) {
      errno = CTORM_ERR_BAD_QUERY;
      goto fail;
    }

    // check the length of the query
    if (str.len > URI_FRAGMENT_MAX) {
      errno = CTORM_ERR_QUERY_TOO_LARGE;
      goto fail;
    }

    // add char to the query
    cu_str_add(&str, *path);
  }

  str.len       = ctorm_percent_decode(str.buf, str.len);
  uri->fragment = str.buf;
  cu_str_clear(&str);
  return path;

fail:
  cu_str_free(&str);
  free(uri->path);
  ctorm_query_free(uri->query);
  free(uri->fragment);

  uri->query       = NULL;
  return uri->path = uri->fragment = NULL;
}
