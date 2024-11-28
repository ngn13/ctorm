#include "../../include/all.h"
#include <string.h>

void handle_notfound(req_t *req, res_t *res) {
  res->code = 404;
  RES_SENDFILE("./example/echo/html/404.html");
}

void handle_post(req_t *req, res_t *res) {
  urlenc_t *form = NULL;
  char     *msg  = NULL;

  if ((form = REQ_FORM()) == NULL) {
    res->code = 400;
    error("Failed to parse the form data: %s", app_geterror());
    return RES_SEND("bad body");
  }

  if (NULL == (msg = enc_url_get(form, "msg"))) {
    res->code = 400;
    enc_url_free(form);
    error("Form data does not contain the message");
    return RES_SEND("bad body");
  }

  info("Message: %s", msg);
  RES_FMT("Message: %s", msg);

  RES_SET("Cool", "yes");

  enc_url_free(form);
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
    error("Failed to start the app: %s", app_geterror());

  // clean up
  app_free(app);
}
