#include "crow_all.h"

int main(){
  // create a simple app
  crow::SimpleApp app;

  // disable logging
  app.loglevel(crow::LogLevel::Warning);

  // setup the hello world route
  CROW_ROUTE(app, "/")([](const crow::request&, crow::response& res){
    res.write("hello world!");
    return res.end();
  });

  // start the app
  app.port(8080).run();
}
