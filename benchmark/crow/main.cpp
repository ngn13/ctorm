#include <crow/app.h>

int main(){
  crow::SimpleApp app;
  app.loglevel(crow::LogLevel::Warning);
  CROW_ROUTE(app, "/")([](const crow::request&, crow::response& res){
    res.write("hello world!");
    return res.end();
  });
  app.port(8080).run();
}
