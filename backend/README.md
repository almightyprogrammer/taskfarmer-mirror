# taskfarmer docs

## Database.hpp
Database layer of taskfarmer. The methods in this header file are to do with CRUD operations and persisting this operations into the database.

In taskfarm

### `std::string db_path_`
A private string field that stores the path to the database file.

### `sqlite3_db* db_`
A private pointer field to our sqlite database connection.

### `explicit Database(std::string db_path)`
An explicit constructor that takes in a path string to the database file.

### `Database(const Database&) = delete`
Deleted copy constructor.

### `Database& operator=(const Database&) = delete`
Deleted copy assignment operator.

### `void open()`
This method is called to set up the database connection to an sqlite3 file. This method also sets some database configs.

### `void close()`
This method is called to close the database connection properly. This method is called by the Database destructor.

### `void init_schema()`
This method initialises all the tables and indexes on the first server start up.

### `std::string ensure_root(std::string root_id, std::string root_title)`
This method has default values for `root_id` and `root_title`, `"ROOT"` and `"/"` respectively.

This method checks that the virtual workspace node exists in the database, and creates one if one does not. The workspace node is a "pseudo" node that acts as a parent to all the projects (root task nodes).

### `TaskNode load_root()`
This method loads the virtual workspace node by value.

### `bool insert_task(const TaskNode& node, const std::string& parent_id)`
This method inserts (creates) a new task in the tasks table and uses `parent_id` to create a relationship with the parent task node.

### `std::optional<TaskNode> get_task_by_id(std::string_view id) const`
This method reads a task row from the database and hydrates and loads a `TaskNode` object into memory.

Since, the task under `id` may not exists, this is why we return a `std::optional<TaskNode>` object.

### `std::vector<TaskNode> list_children(std::string_view parent_id) const`
This method reads, from the database, all task rows that is associated with a parent with `parent_id` and then hydrates and loads it into a memory.

### `bool update_task_field(const TaskNode& node)`
This is method that takes in a `TaskNode` object and persists the current state of the object into the corresponding row in the `tasks` table.

### `bool delete_task_only(std::string_view id)`
Not implemented yet.

### `bool delete_subtree(std::string_view id)`
Not implemented yet.

### `TaskNode::Ptr load_tree(std::string_view root_id)`
`TaskNode::Ptr` is equivalent to `std::shared_ptr<TaskNode>`.

This loads the entire workspace tree into memory by reading from the database.

Once it reads everything, and forms a tree, it returns the shared pointer to the root of the workspace tree.

