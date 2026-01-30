#ifndef TASKFARMER_V2_HTTPSERVER_HPP
#define TASKFARMER_V2_HTTPSERVER_HPP

#include <httplib.h>
#include <string>
#include "TaskService.hpp"

class HttpServer {
public:
    HttpServer(std::string host, int port, TaskService& service);

    void setup_routes();

    void run();

private:
    std::string host_;
    int port_;
    TaskService& service_;

    httplib::Server server_;

    void register_health_endpoint();
    void register_api_endpoint();

    static void set_json(
        httplib::Response& res,
        int status,
        const std::string& body
    );
};

#endif //TASKFARMER_V2_HTTPSERVER_HPP