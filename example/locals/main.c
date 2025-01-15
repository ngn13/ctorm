#include <ctorm.h>

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  char *username = REQ_LOCAL("username");
  RES_FMT("username: %s", username);
}

void username_middleware(ctorm_req_t *req, ctorm_res_t *res) {
  char *username = REQ_QUERY("username");

  if (NULL == username) {
    RES_SEND("no username provided");
    REQ_CANCEL();
    return;
  }

  REQ_LOCAL("username", username);
}

int main() {
  // create the app
  ctorm_app_t *app = ctorm_app_new(NULL);

  // setup the routes
  MIDDLEWARE_GET(app, "/*", username_middleware);
  GET(app, "/", GET_index);

  // run the app
  if (!ctorm_app_run(app, "0.0.0.0:8083"))
    ctorm_fail("failed to start the app: %s", ctorm_geterror());

  // clean up
  ctorm_app_free(app);
}
