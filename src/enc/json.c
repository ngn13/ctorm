#include "enc/json.h"

#if CTORM_JSON_SUPPORT

cJSON *ctorm_json_decode(char *data) {
  return NULL == data ? NULL : cJSON_Parse(data);
}

char *ctorm_json_encode(cJSON *json) {
  return NULL == json ? NULL : cJSON_Print(json);
}

void ctorm_json_free(cJSON *json) {
  if (NULL != json)
    cJSON_Delete(json);
}

#else

cJSON *ctorm_json_decode(char *data) {
  errno = NoJSONSupport;
  return NULL;
}

char *ctorm_json_encode(cJSON *json) {
  errno = NoJSONSupport;
  return NULL;
}

void ctorm_json_free(cJSON *json) {
  errno = NoJSONSupport;
}

#endif
