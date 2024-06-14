#include "../../include/all.h"
#include <string.h>

void handle_notfound(req_t *req, res_t *res) {
  res->code = 404;
  RES_SENDFILE("./example/echo/html/404.html");
}

void handle_post(req_t *req, res_t *res) {
  form_t *form = REQ_FORM();
  if (NULL == form) {
    res->code = 400;
    return RES_SEND("bad body");
  }

  char *msg = req_form_get(form, "msg");
  if (NULL == msg) {
    res->code = 400;
    req_form_free(form);
    return RES_SEND("bad body");
  }

  info("Message: %s", msg);
  RES_FMT("Message: %s", msg);

  RES_SET("Cool", "yes");

  req_form_free(form);
}

void handle_get(req_t *req, res_t *res) {
  if (!RES_SENDFILE("./example/echo/html/index.html"))
    error("Failed to send index.html: %s", app_geterror());
}

int main() {
  // create the app configuration
  app_config_t config;
  app_config_new(&config);

  // create the app
  app_t *app = app_new(&config);

  // setup the routes
  GET(app, "/", handle_get);
  POST(app, "/post", handle_post);

  // setup the static route
  app_static(app, "/static", "./example/echo/static");

  // setup the non handled route handler
  app_all(app, handle_notfound);

  // run the app
  if (!app_run(app, "0.0.0.0:8080"))
    error("app failed: %s\n", app_geterror());

  // clean up
  app_free(app);
}
