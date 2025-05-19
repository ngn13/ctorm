#include "encoding.h"
#include "error.h"

#if CTORM_JSON_SUPPORT

cJSON *ctorm_json_decode(char *data) {
  cJSON *json = cJSON_Parse(data);

  if (NULL == json) {
    errno = CTORM_ERR_JSON_FAIL;
    return NULL;
  }

  return json;
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
  errno = CTORM_ERR_NO_JSON_SUPPORT;
  return NULL;
}

char *ctorm_json_encode(cJSON *json) {
  errno = CTORM_ERR_NO_JSON_SUPPORT;
  return NULL;
}

void ctorm_json_free(cJSON *json) {
  errno = CTORM_ERR_NO_JSON_SUPPORT;
}

#endif
