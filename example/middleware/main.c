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
  RES_SEND("You don't have permission to view this page!");
}

void GET_user_list(ctorm_req_t *req, ctorm_res_t *res) {
  cJSON *json = NULL, *list = NULL, *user, *name, *age;
  json = cJSON_CreateObject();
  list = cJSON_AddArrayToObject(json, "list");

  if (NULL == json || NULL == list) {
    RES_CODE(500);
    RES_SEND("failed to create a JSON");
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
    ctorm_fail("failed to send the JSON data: %s", ctorm_geterror());
    RES_CODE(500);
    RES_SEND("failed to send the JSON");
  }
}

void DELETE_user_delete(ctorm_req_t *req, ctorm_res_t *res) {
  char   *name = REQ_QUERY("name");
  user_t *cur = users, *prev = NULL;

  if (NULL == name) {
    RES_CODE(400);
    return RES_SEND("Please specify a name");
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
    return RES_SEND("Success!");
  }

  RES_CODE(404);
  return RES_SEND("User not found");
}

void POST_user_add(ctorm_req_t *req, ctorm_res_t *res) {
  cJSON *name = NULL, *age = NULL;
  cJSON *json = REQ_JSON();

  if (NULL == json) {
    RES_CODE(400);
    ctorm_fail("failed to get the JSON body: %s", ctorm_geterror());
    return RES_SEND("please specify user data");
  }

  name = cJSON_GetObjectItem(json, "name");
  age  = cJSON_GetObjectItem(json, "age");

  if (NULL == name || NULL == age) {
    RES_CODE(400);
    ctorm_fail("failed to get the name or age");
    return RES_SEND("please specify user data");
  }

  user_t *user = malloc(sizeof(user_t));
  user->name   = cJSON_GetStringValue(name);
  user->age    = cJSON_GetNumberValue(age);

  if (NULL == users) {
    users = user;
    return RES_SEND("success!");
  }

  user_t *cur = users;
  while (NULL != cur) {
    if (NULL == cur->next) {
      cur->next = user;
      break;
    }
    cur = cur->next;
  }

  return RES_SEND("success!");
}

int main() {
  users       = malloc(sizeof(user_t));
  users->next = NULL;
  users->name = "John";
  users->age  = 23;

  ctorm_config_t config;
  ctorm_config_new(&config);

  ctorm_app_t *app = ctorm_app_new(&config);

  MIDDLEWARE_ALL(app, "/user/*", user_auth);
  DELETE(app, "/user/delete", DELETE_user_delete);
  POST(app, "/user/add", POST_user_add);
  GET(app, "/", GET_index);
  GET(app, "/users", GET_user_list);

  if (!ctorm_app_run(app, "0.0.0.0:8084"))
    ctorm_fail("failed to start the app: %s", ctorm_geterror());

  ctorm_app_free(app);
}
