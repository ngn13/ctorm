#include "../../include/encoding.h"
#include "../../include/options.h"

#include <string.h>

cJSON *enc_json_parse(char *data) {
#if CTORM_JSON_SUPPORT
  return NULL == data ? NULL : cJSON_Parse(data);
#else
  errno = NoJSONSupport;
  return NULL;
#endif
}

char *enc_json_dump(cJSON *json, uint64_t *size) {
#if CTORM_JSON_SUPPORT
  char *res = cJSON_Print(json);
  *size     = NULL == res ? 0 : strlen(res);
  return res;
#else
  errno = NoJSONSupport;
  return NULL;
#endif
}

void enc_json_free(cJSON *json) {
#if CTORM_JSON_SUPPORT
  if (NULL != json)
    cJSON_Delete(json);
#else
  errno = NoJSONSupport;
#endif
}
