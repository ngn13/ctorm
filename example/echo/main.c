#include "../../include/macros.h"
#include <string.h>

void handle_notfound(req_t *req, res_t *res) {
  res->code = 404;
  RES_SENDFILE("./example/echo/html/404.html");
}

void handle_post(req_t *req, res_t *res) {
  body_t *body = req_body_parse(req);
  if (NULL == body)
    return RES_SEND("bad body");

  char *msg = req_body_get(body, "msg");
  if (NULL == msg)
    return RES_SEND("bad body");

  info("Message: %s", msg);

  char ret[strlen(msg)+20];
  sprintf(ret, "Message: %s", msg);
  RES_SEND(ret);

  RES_SET("Cool", "yes");
  req_body_free(body);
}

void handle_get(req_t *req, res_t *res) {
  if(!RES_SENDFILE("./example/echo/html/index.html"))
    error("Failed to send index.html: %s", app_geterror());
}

int main() {
  // create the app configuration
  app_config_t config;
  app_config_new(&config);

  // create the app
  app_t *app = app_new(&config);

  // setup the routes
  GET("/", handle_get);
  POST("/post", handle_post);

  // setup the static route
  APP_STATIC("/static", "./example/echo/static");

  // setup the non handled route handler
  APP_ALL(handle_notfound);

  // run the app
  if (!APP_RUN("0.0.0.0:8080"))
    error("app failed: %s\n", app_geterror());

  // clean up
  app_free(app);
}
