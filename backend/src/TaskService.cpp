#include "../include/TaskService.hpp"


#include "../include/TaskService.hpp"

#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

TaskService::TaskService(Database& db) : db_(db) {
    init();
}

void TaskService::init() {
    db_.open();
    db_.init_schema();
    db_.ensure_root();

    workspace_ = db_.load_tree("ROOT");
}

TaskNode::Ptr TaskService::workspace() const {
    std::shared_lock lock(mutex_);
    require_initialised();
    return workspace_;
}

std::vector<TaskNode::Ptr> TaskService::ls(std::string_view absolute_path) const {
    std::shared_lock lock(mutex_);
    require_initialised();

    TaskNode::Ptr parent = resolve_path(workspace_, absolute_path);
    if (!parent) {
        return {};
    }

    const auto& children = parent->get_children();
    return std::vector<TaskNode::Ptr>(children.begin(), children.end());
}

TaskNode::Ptr TaskService::create(std::string_view parent_path,
                                  std::string title,
                                  std::string description) {
    std::unique_lock lock(mutex_);
    require_initialised();

    TaskNode::Ptr parent_ptr = resolve_path(workspace_, parent_path);
    if (!parent_ptr) {
        throw std::runtime_error("[ERROR] create: parent path not found.");
    }

    TaskNode child = db_.create_task_under(
        parent_ptr->get_id(),
        std::move(title),
        std::move(description)
    );

    TaskNode::Ptr child_ptr = std::make_shared<TaskNode>(child);
    parent_ptr->add_child(child_ptr);

    return child_ptr;
}

TaskNode::Ptr TaskService::find(std::string_view absolute_path) const {
    std::shared_lock lock(mutex_);
    require_initialised();
    return resolve_path(workspace_, absolute_path);
}

TaskNode::Ptr TaskService::find_by_id_in_memory(std::string_view id) const {
    require_initialised();

    if (workspace_->get_id() == id) {
        return workspace_;
    }

    std::vector<TaskNode::Ptr> search_space{workspace_};

    while (!search_space.empty()) {
        TaskNode::Ptr current_node = search_space.back();
        search_space.pop_back();

        for (const auto& child : current_node->get_children()) {
            if (!child) {
                continue;
            }

            if (child->get_id() == id) {
                return child;
            }

            search_space.push_back(child);
        }
    }

    return nullptr;
}

bool TaskService::modify(std::string_view id,
                         std::optional<std::string> title,
                         std::optional<std::string> description,
                         std::optional<TaskStatus> status,
                         std::optional<TaskPriority> priority) {
    std::unique_lock lock(mutex_);
    require_initialised();

    TaskNode::Ptr node = find_by_id_in_memory(id);
    if (!node) {
        return false;
    }

    if (node == workspace_) {
        throw std::runtime_error("modify: refusing to modify workspace root");
    }

    if (title) node->set_title(*title);
    if (description) node->set_description(*description);
    if (status) node->set_status(*status);
    if (priority) node->set_priority(*priority);

    const bool ok = db_.update_task_fields(*node);
    if (!ok) {
        throw std::runtime_error("modify: DB update failed");
    }

    return true;
}

void TaskService::require_initialised() const {
    if (!workspace_) {
        throw std::runtime_error(
            "[ERROR] The workspace root node is not initialised."
        );
    }
}



bool TaskService::persist(const TaskNode::Ptr& node) {
    std::unique_lock lock(mutex_);
    return db_.update_task_fields(*node);
}


bool TaskService::delete_subtree(std::string_view id) {
    std::unique_lock lock(mutex_);
    require_initialised();

    if (workspace_->get_id() == id) {
        throw std::runtime_error(
            "delete_subtree: refusing to delete root node"
        );
    }

    TaskNode::Ptr target = find_by_id_in_memory(id);
    if (!target) {
        return false; 
    }

    TaskNode* parent = target->get_parent();
    if (!parent) {
        throw std::runtime_error(
            "delete_subtree: node has no parent"
        );
    }

    const bool db_ok = db_.delete_subtree(id);
    if (!db_ok) {
        throw std::runtime_error(
            "delete_subtree: DB delete failed"
        );
    }

    const bool removed =
        parent->remove_child_by_id(std::string{id});

    if (!removed) {
        throw std::runtime_error(
            "delete_subtree: in-memory removal failed"
        );
    }

    return true;
}


std::vector<TaskNode::Ptr>
TaskService::ls_by_parent_id(std::string_view parent_id) const {
    std::shared_lock lock(mutex_);
    require_initialised();

    // Root case
    if (parent_id == "ROOT") {
        const auto& children = workspace_->get_children();
        return std::vector<TaskNode::Ptr>(children.begin(), children.end());
    }

    // Find parent in memory
    TaskNode::Ptr parent = find_by_id_in_memory(parent_id);
    if (!parent) {
        return {};
    }

    const auto& children = parent->get_children();
    return std::vector<TaskNode::Ptr>(children.begin(), children.end());
}


TaskNode::Ptr TaskService::create_with_parent_id(
    std::string_view parent_id,
    std::string_view title,
    std::string description,
    TaskStatus status,
    TaskPriority priority
) {
    std::unique_lock lock(mutex_);
    require_initialised();

    TaskNode::Ptr parent;

    if (parent_id == "ROOT") {
        parent = workspace_;
    } else {
        parent = find_by_id_in_memory(parent_id);
    }

    if (!parent) {
        throw std::runtime_error(
            "create_with_parent_id: parent not found"
        );
    }

    TaskNode child = db_.create_task_under(
        parent->get_id(),
        std::string{title},
        std::move(description)
    );

    TaskNode::Ptr child_ptr =
        std::make_shared<TaskNode>(std::move(child));

    child_ptr->set_status(status);
    child_ptr->set_priority(priority);

    const bool ok = db_.update_task_fields(*child_ptr);
    if (!ok) {
        throw std::runtime_error(
            "create_with_parent_id: failed to persist fields"
        );
    }

    parent->add_child(child_ptr);

    return child_ptr;
}