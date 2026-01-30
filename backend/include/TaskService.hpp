#ifndef TASKFARMER_V2_TASKSERVICE_HPP
#define TASKFARMER_V2_TASKSERVICE_HPP

#include "Database.hpp"
#include "TaskNode.hpp"

#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
class TaskService {
public:
    explicit TaskService(Database& db);

    void init();

    // Returns the workspace root node.
    TaskNode::Ptr workspace() const;

    // Lists children from an absolute path
    // Returns a vector of shared pointers to the nodes in memory.
    std::vector<TaskNode::Ptr> ls(std::string_view absolute_path) const;

    // Creates a new child tasks node under the parent path.
    // E.g. /A/AA with title AAA, then after successful creation, /A/AA/AAA
    TaskNode::Ptr create(
        std::string_view parent_path,
        std::string title,
        std::string description = "");

    TaskNode::Ptr create_with_parent_id(
        std::string_view parent_id,
        std::string_view title,
        std::string description,
        TaskStatus status,
        TaskPriority priority
    );

    // Returns the pointer to the task node at the absolute path.
    // Returns nullptr if it does not exist.
    TaskNode::Ptr find(std::string_view absolute_path) const;
    TaskNode::Ptr find_by_id_in_memory(std::string_view id) const;
    bool modify(std::string_view id,
                         std::optional<std::string> title,
                         std::optional<std::string> description,
                         std::optional<TaskStatus> status,
                         std::optional<TaskPriority> priority);

    bool persist(const TaskNode::Ptr& node);
    bool delete_subtree(std::string_view id);
    std::vector<TaskNode::Ptr> ls_by_parent_id(std::string_view parent_id) const;

private:
    Database& db_;

    TaskNode::Ptr workspace_;

    mutable std::shared_mutex mutex_;

    void require_initialised() const;

    TaskNode::Ptr resolve(std::string_view absolute_path) const;

};


#endif