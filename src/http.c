#include "http.h"
#include "util.h"
#include "log.h"

#include <stdlib.h>
#include <unistd.h>

// list of HTTP request method descriptions */
struct ctorm_http_method_desc ctorm_http_methods[] = {
    // ------ | ----------- | ----------- |
    // HTTP   | allows body | require bdy |
    // method | req  | res  | req  | res  |
    // ------ | ---- | ---- | ---- | ---- |
    {"GET",     false, true,  false, false},
    {"HEAD",    false, false, false, false},
    {"POST",    true,  true,  true,  false},
    {"PUT",     true,  true,  true,  false},
    {"DELETE",  true,  true,  false, false},
    {"CONNECT", false, false, false, false},
    {"OPTIONS", true,  true,  false, false},
    {"TRACE",   false, true,  false, true },
    // NULL entry marks the end of the list
    {NULL,      false, false, false, false},
};

#define CTORM_HTTP_METHOD_COUNT                                                \
  (sizeof(ctorm_http_methods) / sizeof(ctorm_http_methods[0]))

// is ctorm_http_load() ever called
bool _ctorm_http_loaded = false;

// all the dynamic values
uint32_t ctorm_http_target_max       = 0;
uint32_t ctorm_http_header_name_max  = 0;
uint32_t ctorm_http_header_value_max = 0;

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

bool ctorm_http_version(char *buf, ctorm_http_version_t *version) {
  if (NULL == buf || NULL == version)
    return false;

  if (cu_streq(buf, "HTTP/1.1")) {
    *version = CTORM_HTTP_1_1;
    return true;
  }

  else if (cu_streq(buf, "HTTP/1.0")) {
    *version = CTORM_HTTP_1_0;
    return true;
  }

  return false;
}

bool ctorm_http_method(char *buf, ctorm_http_method_t *method) {
  if (NULL == buf || NULL == method)
    return false;

  struct ctorm_http_method_desc *desc = &ctorm_http_methods[0];

  for (; NULL != desc->name; desc++) {
    if (cu_streq((char *)desc->name, buf)) {
      *method = desc - &ctorm_http_methods[0];
      return true;
    }
  }

  return false;
}

bool ctorm_http_is_valid_header_name(char *name, uint32_t size) {
  if (size > ctorm_http_header_name_max)
    return false;

  // header name should be a token as defined in "3.2. Header Fields"
  for (; size > 0; size--, name++)
    if (!cu_is_letter(*name) && !cu_is_digit(*name) &&
        !cu_contains("!#$%&'*+-.^_`|~", *name))
      return false;

  return true;
}

bool ctorm_http_is_valid_header_value(char *value, uint32_t size) {
  if (size > ctorm_http_header_value_max)
    return false;

  /*

   * header value is defined "3.2. Header Fields" and contain a lot different
   * a lot of different bytes

   * this function does not handle obs-fold, caller should convert obs-fold to
   * spaces as suggested in the RFC (for HTTP requests, this is done in req.c)

  */
  for (; size > 0; size--, value++) {
    // VCHAR (RFC 5234, ABNF)
    if ((uint8_t)*value >= 0x21 && (uint8_t)*value <= 0x7e)
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
