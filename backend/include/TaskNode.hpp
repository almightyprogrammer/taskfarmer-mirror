#ifndef TASKFARMER_V2_TASKNODE_HPP
#define TASKFARMER_V2_TASKNODE_HPP

#include <ctime>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class TaskStatus { TODO, IN_PROGRESS, COMPLETED, BLOCKED };

enum class TaskPriority { LOW, MEDIUM, HIGH, CRITICAL };

// TaskNode is a tree node representing a project/task.
// We will use a "virtual workspace root" node titled "/" whose children are
// top-level projects like "Tetris Clone" and "Sims Clone".
class TaskNode {
public:
    using Ptr = std::shared_ptr<TaskNode>;

private:
    std::string id_;
    std::string title_;
    std::string description_;

    TaskStatus status_ = TaskStatus::TODO;
    TaskPriority priority_ = TaskPriority::MEDIUM;

    std::time_t created_at_ = 0;
    std::time_t updated_at_ = 0;

    TaskNode* parent_ = nullptr;          // non-owning (down-only navigation)
    std::vector<Ptr> children_;           // owning

public:
    explicit TaskNode(std::string title, std::string description = "");
    TaskNode(
        std::string id,
        std::string title,
        std::string description,
        TaskStatus status,
        TaskPriority priority,
        std::time_t created_at,
        std::time_t updated_at
        );
    // Factory: creates a child node under `parent` and wires up parent/child.
    static Ptr create_child(
        const Ptr& parent,
        std::string title,
        std::string description = ""
    );

    // Getters (return by const reference to avoid copies)
    const std::string& get_id() const { return id_; }
    const std::string& get_title() const { return title_; }
    const std::string& get_description() const { return description_; }

    TaskStatus get_status() const { return status_; }
    TaskPriority get_priority() const { return priority_; }
    std::time_t get_created_at() const { return created_at_; }
    std::time_t get_updated_at() const { return updated_at_; }

    TaskNode* get_parent() const { return parent_; }
    const std::vector<Ptr>& get_children() const { return children_; }

    // Mutators
    void set_title(const std::string& title);
    void set_description(const std::string& description);
    void set_status(TaskStatus status);
    void set_priority(TaskPriority priority);

    // Tree operations
    void add_child(const Ptr& child);

    // Returns true if a child was removed, false if not found.
    bool remove_child_by_id(const std::string& id);

    Ptr find_child_by_id(const std::string& id) const;

    // Find direct child by title (name)
    Ptr find_child_by_title(std::string_view title) const;

    // Optional: helpful for debugging / displaying location
    std::string get_path() const;

    bool is_root() const { return parent_ == nullptr; }
    bool has_children() const { return !children_.empty(); }

private:
    void touch();
    static std::string generate_id();
};

// -----------------------------------------------------------------------------
// Option A (we chose this):
// Resolve an absolute path from the workspace root by traversing DOWN only.
//
// Examples:
//   resolve_path(workspace, "/") -> workspace
//   resolve_path(workspace, "/Tetris Clone/UI/") -> node "UI" (if exists)
//   resolve_path(workspace, "/DoesNotExist") -> nullptr
//
// Notes:
// - ignores repeated and trailing slashes
// - does NOT interpret "." or ".."
// -----------------------------------------------------------------------------
TaskNode::Ptr resolve_path(
    const TaskNode::Ptr& workspace_root,
    std::string_view absolute_path
);

#endif