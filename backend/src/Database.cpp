// Database.cpp
#include "../include/Database.hpp"

#include <ctime>
#include <stdexcept>

Database::Database(std::string db_path) : db_path_(std::move(db_path)) {}

Database::~Database() { close(); }

void Database::open() {
    if (db_ != nullptr) {
        return;
    }

    const int rc = sqlite3_open(db_path_.c_str(), &db_);

    if (rc != SQLITE_OK) {
        std::string error_msg{"[ERROR] sqlite3_open failed"};

        if (db_ != nullptr) {
            error_msg += ": ";
            error_msg += sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
        }

        error_msg += ".\n";
        throw std::runtime_error(error_msg);
    }

    exec("PRAGMA foreign_keys = ON;");
    exec("PRAGMA journal_mode = WAL;");
    exec("PRAGMA synchronous = NORMAL;");
    exec("PRAGMA busy_timeout = 3000;");
}

void Database::close() {
    if (db_ == nullptr) {
        return;
    }
    sqlite3_close(db_);
    db_ = nullptr;
}

void Database::exec(std::string_view sql) const {
    if (db_ == nullptr) {
        throw std::runtime_error("[ERROR] Database::exec: db_ is null.\n");
    }

    char* error_msg = nullptr;
    const int rc = sqlite3_exec(db_, sql.data(), nullptr, nullptr, &error_msg);

    if (rc != SQLITE_OK) {
        std::string msg = "[ERROR] Database::exec failed: ";
        if (error_msg) {
            msg += error_msg;
            sqlite3_free(error_msg);
        } else {
            msg += sqlite3_errmsg(db_);
        }
        throw std::runtime_error(msg);
    }
}

void Database::throw_sqlite(sqlite3* db, int rc, std::string_view context) {
    if (rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE) {
        return;
    }

    const char* err = db ? sqlite3_errmsg(db) : "sqlite error";
    throw std::runtime_error(std::string(context) + ": " + err);
}

void Database::init_schema() {
    if (db_ == nullptr) {
        throw std::runtime_error("init_schema: database is not open");
    }

    {
        char* err_msg = nullptr;
        const char* sql = R"sql(
            CREATE TABLE IF NOT EXISTS tasks (
                id          TEXT PRIMARY KEY,
                parent_id   TEXT,
                title       TEXT NOT NULL,
                description TEXT NOT NULL,
                status      INTEGER NOT NULL,
                priority    INTEGER NOT NULL,
                created_at  INTEGER NOT NULL,
                updated_at  INTEGER NOT NULL
            );
        )sql";

        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string msg = "Couldn't initialise schema: ";
            if (err_msg) {
                msg += err_msg;
                sqlite3_free(err_msg);
            } else {
                msg += sqlite3_errmsg(db_);
            }
            throw std::runtime_error(msg);
        }
    }


    {
        char* err_msg = nullptr;
        const char* sql = R"sql(
            CREATE TABLE IF NOT EXISTS users (
                id   TEXT PRIMARY KEY,
                name TEXT NOT NULL
            );
        )sql";

        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string msg = "Couldn't initialise schema: ";
            if (err_msg) {
                msg += err_msg;
                sqlite3_free(err_msg);
            } else {
                msg += sqlite3_errmsg(db_);
            }
            throw std::runtime_error(msg);
        }

    }


    {
        char* err_msg = nullptr;
        const char* sql = R"sql(
            CREATE TABLE IF NOT EXISTS roles (
              name TEXT PRIMARY KEY
            );
            INSERT OR IGNORE INTO roles(name) VALUES
              ('SUPERUSER'),
              ('MANAGER'),
              ('USER'),
              ('DATA_ADMIN');
        )sql";

        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string msg = "Couldn't initialise schema: ";
            if (err_msg) {
                msg += err_msg;
                sqlite3_free(err_msg);
            } else {
                msg += sqlite3_errmsg(db_);
            }
            throw std::runtime_error(msg);
        }
    }

    {
        char* err_msg = nullptr;
        const char* sql = R"sql(
            CREATE TABLE IF NOT EXISTS user_roles (
              user_id   TEXT NOT NULL,
              role_name TEXT NOT NULL,
              PRIMARY KEY (user_id, role_name),
              FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
              FOREIGN KEY (role_name) REFERENCES roles(name) ON DELETE CASCADE
            );
        )sql";

        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string msg = "Couldn't initialise schema: ";
            if (err_msg) {
                msg += err_msg;
                sqlite3_free(err_msg);
            } else {
                msg += sqlite3_errmsg(db_);
            }
            throw std::runtime_error(msg);
        }
    }

    {
        char* err_msg = nullptr;
        const char* sql = R"sql(
            CREATE INDEX IF NOT EXISTS idx_tasks_parent_id
            ON tasks(parent_id);
            CREATE INDEX IF NOT EXISTS idx_user_roles_user_id
            ON user_roles(user_id);
            CREATE INDEX IF NOT EXISTS idx_user_roles_role_name
            ON user_roles(role_name);
        )sql";

        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string msg = "Couldn't create index: ";
            if (err_msg) {
                msg += err_msg;
                sqlite3_free(err_msg);
            } else {
                msg += sqlite3_errmsg(db_);
            }
            throw std::runtime_error(msg);
        }
    }


}

std::string Database::ensure_root(std::string root_id, std::string root_title) {
    open();
    init_schema();

    sqlite3_stmt* stmt = nullptr;

    const char* sql = R"sql(
        INSERT OR IGNORE INTO tasks
            (id, parent_id, title, description, status, priority, created_at,
             updated_at)
        VALUES
            (?, NULL, ?, '', ?, ?, ?, ?);
    )sql";

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    throw_sqlite(db_, rc, "ensure_root: prepare");

    const sqlite3_int64 ts = static_cast<sqlite3_int64>(std::time(nullptr));

    rc = sqlite3_bind_text(stmt, 1, root_id.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind root_id");
    }

    rc = sqlite3_bind_text(stmt, 2, root_title.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind root_title");
    }

    rc = sqlite3_bind_int(stmt, 3, static_cast<int>(TaskStatus::TODO));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind status");
    }

    rc = sqlite3_bind_int(stmt, 4, static_cast<int>(TaskPriority::MEDIUM));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind priority");
    }

    rc = sqlite3_bind_int64(stmt, 5, ts);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind created_at");
    }

    rc = sqlite3_bind_int64(stmt, 6, ts);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: bind updated_at");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "ensure_root: step");
    }

    sqlite3_finalize(stmt);
    return root_id;
}

TaskNode Database::load_root() {
    std::string root_id{ensure_root()};
    std::optional<TaskNode> root_node{get_task_by_id(root_id)};

    if (!root_node.has_value()) {
        throw std::runtime_error(
            "[EsRROR] Tc node does not exist in DB. load_root"
        );
    } else {
        return root_node.value();
    }
}

bool Database::insert_task(const TaskNode& node, const std::string& parent_id) {
    open();
    init_schema();

    sqlite3_stmt* stmt = nullptr;

    const char* sql = R"sql(
        INSERT INTO tasks
            (id, parent_id, title, description, status, priority, created_at, updated_at)
        VALUES
            (?, ?, ?, ?, ?, ?, ?, ?);
    )sql";

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite(db_, rc, "insert task: prepare");
    }

    const sqlite3_int64 ts = static_cast<sqlite3_int64>(std::time(nullptr));

    rc = sqlite3_bind_text(stmt, 1, node.get_id().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "id binding to statement.");
    }

    rc = sqlite3_bind_text(stmt, 2, parent_id.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "parent id binding to statement");
    }

    rc = sqlite3_bind_text(stmt, 3, node.get_title().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "title binding to statement");
    }

    rc = sqlite3_bind_text(stmt, 4, node.get_description().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "description binding to statement");
    }

    rc = sqlite3_bind_int(stmt, 5, static_cast<int>(node.get_status()));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "status binding to statement");
    }

    rc = sqlite3_bind_int(stmt, 6, static_cast<int>(node.get_priority()));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "priority binding to statement");
    }

    rc = sqlite3_bind_int64(stmt, 7, ts);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "created_at binding to statement");
    }

    rc = sqlite3_bind_int64(stmt, 8, ts);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "updated_at binding to statement");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert task: step");
    }

    sqlite3_finalize(stmt);
    return true;
}

std::optional<TaskNode> Database::get_task_by_id(std::string_view id) const {
    if (db_ == nullptr) {
        throw std::runtime_error(
            "[ERROR] Tried to run get_task_by_id but the database uninitialised."
        );
    }

    const char* sql = R"sql(
        SELECT
            id,
            title,
            description,
            status,
            priority,
            created_at,
            updated_at
        FROM
            tasks
        WHERE
            id = ?;
    )sql";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    throw_sqlite(db_, rc, "get_task_by_id prepare stmt");

    rc = sqlite3_bind_text(stmt, 1, id.data(), static_cast<int>(id.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "get_task_by_id bind id");
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        const char* id_text =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* title_text =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* description_text =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const int status_integer = sqlite3_column_int(stmt, 3);
        const int priority_integer = sqlite3_column_int(stmt, 4);
        const std::time_t created_at_time =
            static_cast<std::time_t>(sqlite3_column_int64(stmt, 5));
        const std::time_t updated_at_time =
            static_cast<std::time_t>(sqlite3_column_int64(stmt, 6));

        TaskNode node(
            id_text ? id_text : "",
            title_text ? title_text : "",
            description_text ? description_text : "",
            static_cast<TaskStatus>(status_integer),
            static_cast<TaskPriority>(priority_integer),
            created_at_time,
            updated_at_time
        );

        sqlite3_finalize(stmt);
        return node;
    }

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    sqlite3_finalize(stmt);
    throw_sqlite(db_, rc, "get_task_by_id step.");
    return std::nullopt;
}

/*
 *  This is the N + 1, where this method runs an sql query and we run another sql query.
 *  Pretty Inefficient. So fix this later. Too cbs right now.
 */

std::vector<TaskNode> Database::list_children(std::string_view parent_id) const {
    if (db_ == nullptr) {
        throw std::runtime_error(
            "[ERROR] Tried to run Database::list_children but database is uninitialised."
        );
    }

    const char* sql = R"sql(
        SELECT
            id
        FROM
            tasks
        WHERE
            parent_id = ?;
    )sql";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    throw_sqlite(db_, rc, "preparing sqlite3 statement.");

    rc = sqlite3_bind_text(
        stmt,
        1,
        parent_id.data(),
        static_cast<int>(parent_id.size()),
        SQLITE_TRANSIENT
    );
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding parent_id");
    }

    std::vector<TaskNode> child_nodes{};

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* child_id_text =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        if (!child_id_text) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("[ERROR] Child id does not exist but it should.");
        }

        auto child_node = get_task_by_id(child_id_text);
        if (child_node.has_value()) {
            child_nodes.push_back(child_node.value());
        }
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "The sql statement was not finished. list_children.");
    }

    sqlite3_finalize(stmt);
    return child_nodes;
}

bool Database::update_task_fields(const TaskNode& node) {
    if (db_ == nullptr) {
        throw std::runtime_error(
            "[ERROR] Tried to run update_task_fields but database is uninitialised."
        );
    }

    const char* sql = R"sql(
        UPDATE
            tasks
        SET
            title = ?,
            description = ?,
            status = ?,
            priority = ?,
            updated_at = ?
        WHERE
            id = ?;
    )sql";

    sqlite3_stmt* stmt = nullptr;

    const std::string& node_title = node.get_title();
    const std::string& node_description = node.get_description();
    TaskStatus node_status = node.get_status();
    TaskPriority node_priority = node.get_priority();
    std::time_t node_updated_at = node.get_updated_at();
    const std::string& node_id = node.get_id();

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    throw_sqlite(db_, rc, "preparing sql statement.");

    rc = sqlite3_bind_text(
        stmt,
        1,
        node_title.c_str(),
        static_cast<int>(node_title.size()),
        SQLITE_TRANSIENT
    );
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node title");
    }

    rc = sqlite3_bind_text(
        stmt,
        2,
        node_description.c_str(),
        static_cast<int>(node_description.size()),
        SQLITE_TRANSIENT
    );
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node description");
    }

    rc = sqlite3_bind_int(stmt, 3, static_cast<int>(node_status));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node status");
    }

    rc = sqlite3_bind_int(stmt, 4, static_cast<int>(node_priority));
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node priority");
    }

    rc = sqlite3_bind_int64(
        stmt,
        5,
        static_cast<sqlite3_int64>(node_updated_at)
    );
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node updated_at time");
    }

    rc = sqlite3_bind_text(
        stmt,
        6,
        node_id.c_str(),
        static_cast<int>(node_id.size()),
        SQLITE_TRANSIENT
    );
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "binding node id");
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "[ERROR] Update task fields not completed.");
    }

    // ✅ IMPORTANT FIX:
    // sqlite3_changes == 0 does NOT mean "row not found".
    // It means "no values changed".
    // A successful UPDATE should be treated as success.
    sqlite3_finalize(stmt);
    return true;
}

TaskNode::Ptr Database::load_tree(std::string_view root_id) {
    auto root_opt = get_task_by_id(root_id);
    if (!root_opt.has_value()) {
        throw std::runtime_error("[ERROR] load_tree: node id not found in DB.");
    }

    TaskNode::Ptr root_ptr = std::make_shared<TaskNode>(root_opt.value());

    std::vector<TaskNode> children = list_children(root_id);
    for (const auto& child : children) {
        TaskNode::Ptr child_ptr = load_tree(child.get_id());
        root_ptr->add_child(child_ptr);
    }

    return root_ptr;
}

TaskNode Database::create_task_under(
    std::string_view parent_id,
    std::string title,
    std::string description
) {
    TaskNode node = TaskNode(title, description);
    insert_task(node, std::string(parent_id));
    return node;
}


bool Database::insert_user_role(
    std::string_view user_id,
    std::string_view role_string
) {
    const char* sql = R"sql(
        INSERT INTO user_roles
            (user_id, role_name)
        VALUES
            (?, ?);
    )sql";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite(db_, rc, "insert user role: prepare");
    }

    rc = sqlite3_bind_text(stmt, 1, user_id.data(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "user id binding to statement: insert user role");
    }

    rc = sqlite3_bind_text(stmt, 2, role_string.data(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "role string binding to statement");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert user role: step");
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::insert_user(std::string_view id, std::string_view name) {

    const char* sql = R"sql(
        INSERT INTO users
            (id, name)
        VALUES
            (?, ?);
    )sql";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert_user: prepare");
    }

    rc = sqlite3_bind_text(stmt, 1, id.data(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert_user: binding id");
    }

    rc = sqlite3_bind_text(stmt, 2, name.data(), -1, SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert_user: binding name");
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw_sqlite(db_, rc, "insert_user: step");
    }

    sqlite3_finalize(stmt);
    return true;
}




	bool Database::delete_subtree(std::string_view id) {
	    if (db_ == nullptr) {
	        throw std::runtime_error(
	            "[ERROR] Tried to run delete_subtree but database is uninitialised."
	        );
	    }
	
	    const char* sql = R"sql(
	        WITH RECURSIVE subtree(id) AS (
	            SELECT id FROM tasks WHERE id = ?
	            UNION ALL
	            SELECT tasks.id
	            FROM tasks
	            JOIN subtree ON tasks.parent_id = subtree.id
	        )
	        DELETE FROM tasks WHERE id IN (SELECT id FROM subtree);
	    )sql";
	
	    sqlite3_stmt* stmt = nullptr;
	
	    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	    if (rc != SQLITE_OK) {
	        sqlite3_finalize(stmt);
	        throw_sqlite(db_, rc, "delete_subtree: prepare");
	    }
	
	    rc = sqlite3_bind_text(
	        stmt,
	        1,
	        id.data(),
	        static_cast<int>(id.size()),
	        SQLITE_TRANSIENT
	    );
	    if (rc != SQLITE_OK) {
	        sqlite3_finalize(stmt);
	        throw_sqlite(db_, rc, "delete_subtree: bind id");
	    }
	
	    rc = sqlite3_step(stmt);
	    if (rc != SQLITE_DONE) {
	        sqlite3_finalize(stmt);
	        throw_sqlite(db_, rc, "delete_subtree: step");
	    }
	
	    const bool deleted = sqlite3_changes(db_) > 0;
	
	    sqlite3_finalize(stmt);
	    return deleted;
	}