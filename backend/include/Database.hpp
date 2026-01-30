#ifndef TASKFARMER_V2_DATABASE_HPP
#define TASKFARMER_V2_DATABASE_HPP

#include "TaskNode.hpp"

#include <sqlite3.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "User.hpp"

class Database {
public:
    explicit Database(std::string db_path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Connection + schema
    void open();
    void close();
    void init_schema();

    // Guarantees the existence of the root row in the DB and returns its id
    // (default "ROOT"). This may create the schema if needed.
    std::string ensure_root(std::string root_id = "ROOT",
                            std::string root_title = "/");

    // Loads the root node (by value) from the DB.
    // Requires that the DB can be opened; this function calls ensure_root().
    TaskNode load_root();

    // Persists a TaskNode under the given parent_id.
    bool insert_task(const TaskNode& node, const std::string& parent_id);

    // Retrieves a row by id (hydrated TaskNode).
    // Note: these are const and assume the DB is already open (db_ != nullptr).
    std::optional<TaskNode> get_task_by_id(std::string_view id) const;

    // Lists direct children of the given parent id.
    std::vector<TaskNode> list_children(std::string_view parent_id) const;

    // Updates mutable fields for the given node (matched by node.get_id()).
    bool update_task_fields(const TaskNode& node);

    // Delete operations (to be implemented later)
    bool delete_task_only(std::string_view id);
    bool delete_subtree(std::string_view id);

    // Loads the entire task tree structure into memory, starting from root_id.
    // Typical usage: ensure_root(); auto root = load_tree("ROOT");
    TaskNode::Ptr load_tree(std::string_view root_id = "ROOT");

    // Helper: creates TaskNode in memory and persists it under parent_id.
    TaskNode create_task_under(std::string_view parent_id,
                               std::string title,
                               std::string description = "");


    bool insert_user_role(std::string_view user_id, std::string_view role_string);

    bool insert_user(std::string_view id, std::string_view name);


private:
    std::string db_path_;
    sqlite3* db_ = nullptr;

    void exec(std::string_view sql) const;

    static void throw_sqlite(sqlite3* db, int rc, std::string_view context);

    // Not used in current implementation (load_tree does recursion inline).
    // void load_children_recursive(const TaskNode::Ptr& parent) const;
};

#endif