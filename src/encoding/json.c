#include "encoding.h"
#include <string.h>

#if CTORM_JSON_SUPPORT

cJSON *enc_json_parse(char *data) {
  return NULL == data ? NULL : cJSON_Parse(data);
}

char *enc_json_dump(cJSON *json, uint64_t *size) {
  char *res = cJSON_Print(json);
  *size     = NULL == res ? 0 : strlen(res);
  return res;
}

void enc_json_free(cJSON *json) {
  if (NULL != json)
    cJSON_Delete(json);
}

#else

cJSON *enc_json_parse(char *data) {
  errno = NoJSONSupport;
  return NULL;
}

char *enc_json_dump(cJSON *json, uint64_t *size) {
  errno = NoJSONSupport;
  return NULL;
}

void enc_json_free(cJSON *json) {
  errno = NoJSONSupport;
}

#endif
