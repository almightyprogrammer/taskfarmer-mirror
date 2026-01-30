#include "../include/HttpServer.hpp"

#include <nlohmann/json.hpp>

using nlohmann::json;

HttpServer::HttpServer(std::string host, int port, TaskService& service)
    : host_(std::move(host)), port_(port), service_(service) {}

void HttpServer::set_json(httplib::Response& res, int status,
                          const std::string& body) {
    res.status = status;
    res.set_content(body, "application/json");
}

void HttpServer::register_api_endpoint() {
    // GET /api/ls?path=/Tetris%20Clone/Game%20Logic/
    server_.Get("/api/ls",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                if (!req.has_param("parent_id")) {
                    json j = {{"error", "missing required query param: parent_id"}};
                    return set_json(res, 400, j.dump());
                }

                const std::string parent_id = req.get_param_value("parent_id");

                const auto children = service_.ls_by_parent_id(parent_id);

                json out = json::array();
                for (const auto& child : children) {
                    out.push_back({
                        {"id", child->get_id()},
                        {"title", child->get_title()},
                        {"description", child->get_description()},
                        {"status", child->get_status()},
                        {"priority", child->get_priority()},
                        {"created_at", child->get_created_at()},
                        {"last_updated_at", child->get_updated_at()}
                    });
                }

                return set_json(res, 200, out.dump());
            } catch (const std::exception& e) {
                json j = {{"error", e.what()}};
                return set_json(res, 500, j.dump());
            }
        }
    );

    // POST /api/create
    // Body:
    // { "path": "/Tetris Clone/Game Logic/", "title": "Collision", "description": "...", "priority": 2 }
        server_.Post(
        "/api/create",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body;

                try {
                    body = json::parse(req.body);
                } catch (...) {
                    return set_json(
                        res,
                        400,
                        json{{"error", "invalid JSON body"}}.dump()
                    );
                }

                if (!body.contains("parent_id") || !body["parent_id"].is_string()) {
                    return set_json(
                        res,
                        400,
                        json{{"error", "missing/invalid field: parent_id"}}.dump()
                    );
                }

                if (!body.contains("title") || !body["title"].is_string()) {
                    return set_json(
                        res,
                        400,
                        json{{"error", "missing/invalid field: title"}}.dump()
                    );
                }

                const std::string parent_id =
                    body["parent_id"].get<std::string>();
                const std::string title =
                    body["title"].get<std::string>();
                const std::string description =
                    body.value("description", std::string{""});

                const TaskStatus status =
                    body.contains("status") && body["status"].is_number_integer()
                        ? static_cast<TaskStatus>(body["status"].get<int>())
                        : TaskStatus::TODO;

                const TaskPriority priority =
                    body.contains("priority") && body["priority"].is_number_integer()
                        ? static_cast<TaskPriority>(body["priority"].get<int>())
                        : TaskPriority::MEDIUM;

                TaskNode::Ptr created = service_.create_with_parent_id(
                    parent_id,
                    title,
                    description,
                    status,
                    priority
                );

                json out = {
                    {"id", created->get_id()},
                    {"title", created->get_title()},
                    {"description", created->get_description()},
                    {"status", created->get_status()},
                    {"priority", created->get_priority()},
                    {"created_at", created->get_created_at()},
                    {"last_updated_at", created->get_updated_at()}
                };

                return set_json(res, 201, out.dump());
            } catch (const std::exception& e) {
                return set_json(
                    res,
                    500,
                    json{{"error", e.what()}}.dump()
                );
            }
        }
    );

        server_.Patch(
    "/api/modify",
    [this](const httplib::Request& req, httplib::Response& res) {
        try {
        json body;

        try {
            body = json::parse(req.body);
        } catch (...) {
            return set_json(res, 400, json{{"error", "invalid JSON"}}.dump());
        }

        if (!body.contains("id") || !body["id"].is_string()) {
            return set_json(
            res,
            400,
            json{{"error", "missing/invalid field: id"}}.dump()
            );
        }

        const std::string id = body["id"].get<std::string>();

        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<TaskStatus> status;
        std::optional<TaskPriority> priority;

        if (body.contains("title") && body["title"].is_string())
            title = body["title"].get<std::string>();

        if (body.contains("description") && body["description"].is_string())
            description = body["description"].get<std::string>();

        if (body.contains("status") && body["status"].is_number_integer())
            status = static_cast<TaskStatus>(body["status"].get<int>());

        if (body.contains("priority") && body["priority"].is_number_integer())
            priority = static_cast<TaskPriority>(body["priority"].get<int>());

        const bool ok = service_.modify(
            id,
            title,
            description,
            status,
            priority
        );

        if (!ok) {
            return set_json(
            res,
            404,
            json{{"error", "node not found"}}.dump()
            );
        }

        return set_json(res, 200, json{{"ok", true}}.dump());
        } catch (const std::exception& e) {
        return set_json(
            res,
            500,
            json{{"error", e.what()}}.dump()
        );
        }
    }
    );
    // DELETE /api/delete
    // Body:
    // { "id": "<task-id>" }
    server_.Delete("/api/delete",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body;
                try {
                    body = json::parse(req.body);
                } catch (...) {
                    json j = {{"error", "invalid JSON body"}};
                    return set_json(res, 400, j.dump());
                }

                if (!body.contains("id") || !body["id"].is_string()) {
                    json j = {{"error", "missing/invalid field: id"}};
                    return set_json(res, 400, j.dump());
                }

                const std::string id = body["id"].get<std::string>();

                bool deleted = false;
                try {
                    deleted = service_.delete_subtree(id);
                } catch (const std::runtime_error& e) {

                    std::string msg = e.what();

                    if (msg.find("refusing to delete root") != std::string::npos) {
                        json j = {{"error", "cannot delete root node"}};
                        return set_json(res, 403, j.dump());
                    }

                    throw;
                }

                if (!deleted) {
                    json j = {{"error", "task not found"}};
                    return set_json(res, 404, j.dump());
                }

                json ok = {{"ok", true}};
                return set_json(res, 200, ok.dump());

            } catch (const std::exception& e) {
                json j = {{"error", e.what()}};
                return set_json(res, 500, j.dump());
            }
        }
    );
}

void HttpServer::setup_routes() {
    register_health_endpoint();
    register_api_endpoint();
}

void HttpServer::run() {
    setup_routes();

    server_.set_logger([](
        const httplib::Request& req,
        const httplib::Response& res
    ) {
        fprintf(stdout, "%s %s -> %d\n", req.method.c_str(),
            req.path.c_str(), res.status
        );
    });

    server_.listen(host_.c_str(), port_);
}

void HttpServer::register_health_endpoint() {
    server_.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK\n", "text/plain");
        res.status = 200;
    });


    server_.Get("/echo", [](const httplib::Request& req,
                            httplib::Response& res) {
        std::string msg;
        if (req.has_param("msg")) {
            msg = req.get_param_value("msg");
        } else {
            msg = "(missing msg)";
        }

        res.set_content("echo: " + msg + "\n", "text/plain");
        res.status = 200;
    });
}