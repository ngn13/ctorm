#include "http.h"
#include "util.h"

#include <stdlib.h>
#include <unistd.h>

// describes a single HTTP request method
struct ctorm_http_method_desc {
  const char *name;
  bool        supported;

  bool allows_req_body;
  bool allows_res_body;

  bool requires_req_body;
  bool requires_res_body;
};

// list of HTTP request method descriptions */
struct ctorm_http_method_desc _ctorm_http_methods[] = {
    // ------ | ---- | ----------- | ----------- |
    // HTTP   | sup- | allows body | require bdy |
    // method | port | req  | res  | req  | res  |
    // ------ | ---- | ---- | ---- | ---- | ---- |
    {"GET",     true,  false, true,  false, false},
    {"HEAD",    true,  false, false, false, false},
    {"POST",    true,  true,  true,  true,  false},
    {"PUT",     true,  true,  true,  true,  false},
    {"DELETE",  true,  true,  true,  false, false},
    {"CONNECT", false, false, false, false, false},
    {"OPTIONS", true,  true,  true,  false, false},
    {"TRACE",   true,  false, true,  false, true },
    // NULL entry marks the end of the list
    {NULL,      false, false, false, false, false},
};

#define CTORM_HTTP_METHOD_COUNT                                                \
  (sizeof(_ctorm_http_methods) / sizeof(_ctorm_http_methods[0]))

// is ctorm_http_load() ever called
bool _ctorm_http_loaded = false;

// all the dynamic values
uint64_t ctorm_http_target_max       = 0;
uint64_t ctorm_http_header_name_max  = 0;
uint64_t ctorm_http_header_value_max = 0;

void ctorm_http_load() {
  // check if ctorm_http_load() is already called
  if (_ctorm_http_loaded)
    return;

  ctorm_http_target_max       = getpagesize();
  ctorm_http_header_name_max  = getpagesize();
  ctorm_http_header_value_max = getpagesize() * 4;

  // all the dynamic HTTP are now loaded
  _ctorm_http_loaded = true;
}

ctorm_http_version_t ctorm_http_version(char *version) {
  if (NULL == version)
    return -1;

  if (cu_streq(version, "HTTP/1.1"))
    return CTORM_HTTP_1_1;
  else if (cu_streq(version, "HTTP/1.0"))
    return CTORM_HTTP_1_0;

  return -1;
}

ctorm_http_method_t ctorm_http_method(char *method) {
  if (NULL == method)
    return -1;

  struct ctorm_http_method_desc *desc = &_ctorm_http_methods[0];
  ctorm_http_method_t            num  = 0;

  for (; NULL != desc->name; desc++)
    if (cu_streq(desc->name, method))
      return num;

  return -1;
}

bool ctorm_http_method_supported(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].supported;
}

const char *ctorm_http_method_name(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].name;
}

bool ctorm_http_method_allows_req_body(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].allows_req_body;
}

bool ctorm_http_method_allows_res_body(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].allows_res_body;
}

bool ctorm_http_method_needs_req_body(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].requires_req_body;
}

bool ctorm_http_method_needs_res_body(ctorm_http_method_t method) {
  return _ctorm_http_methods[method].requires_res_body;
}

bool ctorm_http_is_valid_header_name(char *name, uint64_t size) {
  if (ctorm_http_header_name_max > size)
    return false;

  // header name should be a token as defined in "3.2. Header Fields"
  for (; size > 0; size--, name++)
    if (!cu_is_letter(*name) && !cu_is_digit(*name) &&
        !cu_contains("!#$%&'*+-.^_`|~", *name))
      return false;

  return true;
}

bool ctorm_http_is_valid_header_value(char *value, uint64_t size) {
  if (ctorm_http_header_value_max > size)
    return false;

  /*

   * header value is defined "3.2. Header Fields" and contain a lot different
   * a lot of different bytes

   * this function does not handle obs-fold, caller should convert obs-fold to
   * spaces as suggested in the RFC (for HTTP requests, this is done in req.c)

  */
  for (; size > 0; size--, value++) {
    // VCHAR (RFC 5234, ABNS)
    if (*value >= 0x21 && *value <= 0x7e)
      continue;

    // obs-text ("Appendix B. Collected ABNF")
    if ((uint8_t)*value >= 0x80)
      continue;

    // SP / HTAB
    if (*value == ' ' || *value == '\t')
      continue;

    return false;
  }

  return true;
}
