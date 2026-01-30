#include "include/Database.hpp"
#include "include/HttpServer.hpp"
#include "include/TaskService.hpp"

#include <iostream>

int main() {
      try {
            Database db("taskfarmer.db");
            TaskService service(db);
            service.init();

            const std::string host = "0.0.0.0";
            const int port = 8080;

            std::cout << "Starting HttpServer on " << host << ":" << port << "\n";
            HttpServer server(host, port, service);
            server.run();
      } catch (const std::exception& e) {
            std::cerr << "Fatal: " << e.what() << "\n";
            return 1;
      }

      return 0;
}