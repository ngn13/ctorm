#include "../../include/all.h"

void handle_get(req_t *req, res_t *res) {
  RES_SEND("Hello world!");
}

int main() {
  // create the app configuration
  app_config_t config;
  app_config_new(&config);

  // example: disable the server header
  config.server_header = false;

  // create the app
  app_t *app = app_new(&config);

  // setup the routes
  GET(app, "/", handle_get);

  // run the app
  if (!app_run(app, "0.0.0.0:8080"))
    error("Failed to start the app: %s", app_geterror());

  // clean up
  app_free(app);
}
