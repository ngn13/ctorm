#include "../../include/all.h"
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

typedef struct user {
  struct user *next;
  char        *name;
  short        age;
} user_t;

user_t *users = NULL;

void index_redirect(req_t *req, res_t *res){
  RES_REDIRECT("/users");
}

void user_auth(req_t *req, res_t *res) {
  char *auth = REQ_HEADER("Authorization");
  if (NULL != auth && strcmp(auth, "secretpassword") == 0)
    return;

  req->cancel = true;
  res->code   = 403;
  RES_SEND("You don't have permission to view this page!");
}

void user_list(req_t *req, res_t *res) {
  cJSON *json = NULL, *list = NULL, *user, *name, *age;
  json = cJSON_CreateObject();
  list = cJSON_AddArrayToObject(json, "list");

  if (NULL == json || NULL == list) {
    res->code = 500;
    RES_SEND("Failed to create a JSON");
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

  RES_JSON(json);
}

void user_delete(req_t *req, res_t *res) {
  char *name = REQ_QUERY("name");
  if (NULL == name) {
    res->code = 400;
    return RES_SEND("Please specify a name");
  }

  user_t *cur = users, *prev = NULL;
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

  res->code = 404;
  return RES_SEND("User not found");
}

void user_add(req_t *req, res_t *res) {
  cJSON *name = NULL, *age = NULL;
  cJSON *json = REQ_JSON();

  if (NULL == json) {
    res->code = 400;
    return RES_SEND("Please specify user data");
  }

  name = cJSON_GetObjectItem(json, "name");
  age  = cJSON_GetObjectItem(json, "age");

  if (NULL == name || NULL == age) {
    res->code = 400;
    return RES_SEND("Please specify user data");
  }

  user_t *user = malloc(sizeof(user_t));
  user->name   = cJSON_GetStringValue(name);
  user->age    = cJSON_GetNumberValue(age);

  if (NULL == users) {
    users = user;
    return RES_SEND("Success!");
  }

  user_t *cur = users;
  while (NULL != cur) {
    if (NULL == cur->next) {
      cur->next = user;
      break;
    }
    cur = cur->next;
  }

  return RES_SEND("Success!");
}

int main() {
  users       = malloc(sizeof(user_t));
  users->next = NULL;
  users->name = "John";
  users->age  = 23;

  app_config_t config;
  app_config_new(&config);

  app_t *app = app_new(&config);

  MIDDLEWARE_ALL("/user/*", user_auth);
  GET("/", index_redirect);
  GET("/users", user_list);
  DELETE("/user/delete", user_delete);
  POST("/user/add", user_add);

  if (!APP_RUN("0.0.0.0:8080"))
    error("app failed: %s\n", app_geterror());

  app_free(app);
}
