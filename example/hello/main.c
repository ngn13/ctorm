#include <ctorm.h>

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  RES_SEND("hello world!");
}

int main() {
  // create the app configuration
  ctorm_config_t config;
  ctorm_config_new(&config);

  // example: disable the server header
  config.server_header = false;

  // another example: disable request logging
  config.disable_logging = true;

  // create the app
  ctorm_app_t *app = ctorm_app_new(&config);

  // setup the routes
  GET(app, "/", GET_index);

  // run the app
  if (!ctorm_app_run(app, "0.0.0.0:8080"))
    ctorm_fail("failed to start the app: %s", ctorm_geterror());

  // clean up
  ctorm_app_free(app);
}
