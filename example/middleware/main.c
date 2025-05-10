#include <cjson/cJSON.h>
#include <ctorm.h>
#include <stdlib.h>
#include <string.h>

typedef struct user {
  struct user *next;
  char        *name;
  short        age;
} user_t;

user_t *users = NULL;

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  RES_REDIRECT("/users");
}

void user_auth(ctorm_req_t *req, ctorm_res_t *res) {
  char *auth = REQ_GET("authorization");

  if (NULL != auth && strcmp(auth, "secretpassword") == 0)
    return;

  req->cancel = true;
  RES_CODE(403);
  RES_BODY("you don't have permission to view this page!");
}

void GET_user_list(ctorm_req_t *req, ctorm_res_t *res) {
  cJSON *json = NULL, *list = NULL, *user, *name, *age;
  json = cJSON_CreateObject();
  list = cJSON_AddArrayToObject(json, "list");

  if (NULL == json || NULL == list) {
    RES_CODE(500);
    RES_BODY("failed to create a JSON");
  }

  user_t *cur = users;
  while (NULL != cur) {
    user = cJSON_CreateObject();
    cJSON_AddItemToArray(list, user);

    name = cJSON_CreateString(cur->name);
    age  = cJSON_CreateNumber(cur->age);
    cJSON_AddItemToObject(user, "name", name);
    cJSON_AddItemToObject(user, "age", age);

    cur = cur->next;
  }

  if (!RES_JSON(json)) {
    ctorm_fail("failed to send the JSON data: %s", ctorm_error());
    RES_CODE(500);
    RES_BODY("failed to send the JSON");
  }
}

void DELETE_user_delete(ctorm_req_t *req, ctorm_res_t *res) {
  char   *name = REQ_QUERY("name");
  user_t *cur = users, *prev = NULL;

  if (NULL == name) {
    RES_CODE(400);
    RES_BODY("please specify a name");
    return;
  }

  while (NULL != cur) {
    if (strcmp(cur->name, name) != 0) {
      prev = cur;
      cur  = cur->next;
      continue;
    }

    if (NULL == prev)
      users = cur->next;
    else
      prev->next = cur->next;

    free(cur);
    RES_BODY("success!");
    return;
  }

  RES_CODE(404);
  RES_BODY("user not found");
}

void POST_user_add(ctorm_req_t *req, ctorm_res_t *res) {
  cJSON *name = NULL, *age = NULL;
  cJSON *json = REQ_JSON();

  if (NULL == json) {
    RES_CODE(400);
    ctorm_fail("failed to get the JSON body: %s", ctorm_error());
    RES_BODY("please specify user data");
    return;
  }

  name = cJSON_GetObjectItem(json, "name");
  age  = cJSON_GetObjectItem(json, "age");

  if (NULL == name || NULL == age) {
    RES_CODE(400);
    ctorm_fail("failed to get the name or age");
    RES_BODY("please specify user data");
    return;
  }

  user_t *user = malloc(sizeof(user_t));
  user->name   = cJSON_GetStringValue(name);
  user->age    = cJSON_GetNumberValue(age);

  if (NULL == users) {
    users = user;
    RES_BODY("success!");
    return;
  }

  user_t *cur = users;
  while (NULL != cur) {
    if (NULL == cur->next) {
      cur->next = user;
      break;
    }
    cur = cur->next;
  }

  RES_BODY("success!");
}

int main() {
  users       = malloc(sizeof(user_t));
  users->next = NULL;
  users->name = "John";
  users->age  = 23;

  ctorm_config_t config;
  ctorm_config_new(&config);

  ctorm_app_t *app = ctorm_app_new(&config);

  GET(app, "/", GET_index);

  ALL(app, "/user/*", user_auth);
  DELETE(app, "/user/delete", DELETE_user_delete);
  POST(app, "/user/add", POST_user_add);
  GET(app, "/users", GET_user_list);

  if (!ctorm_app_run(app, "0.0.0.0:8084"))
    ctorm_fail("failed to start the app: %s", ctorm_error());

  ctorm_app_free(app);
}
