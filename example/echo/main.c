#include <ctorm.h>
#include <string.h>

void GET_notfound(ctorm_req_t *req, ctorm_res_t *res) {
  RES_CODE(404);
  RES_SENDFILE("./example/echo/html/404.html");
}

void POST_form(ctorm_req_t *req, ctorm_res_t *res) {
  enc_url_t *form = NULL;
  char      *msg  = NULL;

  if ((form = REQ_FORM()) == NULL) {
    RES_CODE(400);
    ctorm_fail("failed to parse the form data: %s", ctorm_geterror());
    return RES_SEND("bad body");
  }

  if (NULL == (msg = enc_url_get(form, "msg"))) {
    RES_CODE(400);
    enc_url_free(form);
    ctorm_fail("form data does not contain the message");
    return RES_SEND("bad body");
  }

  ctorm_info("message: %s", msg);
  RES_FMT("message: %s", msg);

  RES_SET("cool", "yes");

  enc_url_free(form);
}

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  if (!RES_SENDFILE("./example/echo/html/index.html"))
    ctorm_fail("failed to send index.html: %s", ctorm_geterror());
}

int main() {
  // create the app configuration
  ctorm_config_t config;
  ctorm_config_new(&config);

  // create the app
  ctorm_app_t *app = ctorm_app_new(&config);

  // setup the routes
  GET(app, "/", GET_index);
  POST(app, "/post", POST_form);

  // setup the static route
  ctorm_app_static(app, "/static", "./example/echo/static");

  // setup the non handled route handler
  ctorm_app_all(app, GET_notfound);

  // run the app
  if (!ctorm_app_run(app, "0.0.0.0:8080"))
    ctorm_fail("failed to start the app: %s", ctorm_geterror());

  // clean up
  ctorm_app_free(app);
}
