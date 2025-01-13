#include "encoding.h"
#include "util.h"

#if CTORM_JSON_SUPPORT

cJSON *ctorm_json_parse(char *data) {
  return NULL == data ? NULL : cJSON_Parse(data);
}

char *ctorm_json_dump(cJSON *json, uint64_t *size) {
  char *res = cJSON_Print(json);
  *size     = NULL == res ? 0 : cu_strlen(res);
  return res;
}

void ctorm_json_free(cJSON *json) {
  if (NULL != json)
    cJSON_Delete(json);
}

#else

cJSON *ctorm_json_parse(char *data) {
  errno = NoJSONSupport;
  return NULL;
}

char *ctorm_json_dump(cJSON *json, uint64_t *size) {
  errno = NoJSONSupport;
  return NULL;
}

void ctorm_json_free(cJSON *json) {
  errno = NoJSONSupport;
}

#endif
