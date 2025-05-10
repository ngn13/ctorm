#include <ctorm.h>

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  RES_REDIRECT("/echo/changeme/nothing");
}

void GET_param(ctorm_req_t *req, ctorm_res_t *res) {
  RES_FMT("param: %s", REQ_PARAM("param"));
}

int main() {
  // create the app
  ctorm_app_t *app = ctorm_app_new(NULL);

  // setup the routes
  GET(app, "/", GET_index);
  GET(app, "/echo/:param/*", GET_param);

  // run the app
  if (!ctorm_app_run(app, "0.0.0.0:8082"))
    ctorm_fail("failed to start the app: %s", ctorm_error());

  // clean up
  ctorm_app_free(app);
}
