// TaskNode.cpp
#include "../include/TaskNode.hpp"

#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {


std::vector<std::string_view> split_path(std::string_view path) {
    std::vector<std::string_view> parts;

    while (!path.empty()) {
        while (!path.empty() && path.front() == '/') {
            path.remove_prefix(1);
        }
        if (path.empty()) {
            break;
        }

        const auto slash = path.find('/');
        if (slash == std::string_view::npos) {
            parts.push_back(path);
            break;
        }

        const auto seg = path.substr(0, slash);
        if (!seg.empty()) {
            parts.push_back(seg);
        }
        path.remove_prefix(slash + 1);
    }

    return parts;
}

} 

TaskNode::TaskNode(std::string title, std::string description)
    : id_(generate_id()),
      title_(std::move(title)),
      description_(std::move(description)),
      created_at_(std::time(nullptr)),
      updated_at_(created_at_) {}

TaskNode::TaskNode(
    std::string id,
    std::string title,
    std::string description,
    TaskStatus status,
    TaskPriority priority,
    std::time_t created_at,
    std::time_t updated_at
) : id_(std::move(id)),
    title_(std::move(title)),
    description_(std::move(description)),
    status_(status),
    priority_(priority),
    created_at_(created_at),
    updated_at_(updated_at),
    parent_(nullptr),
    children_() {
    if (id_.empty()) {
        throw std::invalid_argument("[ERROR] TaskNode hydration: id is empty");
    }
}

TaskNode::Ptr TaskNode::create_child(
    const Ptr& parent,
    std::string title,
    std::string description
) {
    if (!parent) {
        throw std::invalid_argument("TaskNode::create_child: parent is null");
    }

    auto child =
        std::make_shared<TaskNode>(std::move(title), std::move(description));
    child->parent_ = parent.get();

    parent->children_.push_back(child);
    parent->touch();

    return child;
}

void TaskNode::set_title(const std::string& title) {
    title_ = title;
    touch();
}

void TaskNode::set_description(const std::string& description) {
    description_ = description;
    touch();
}

void TaskNode::set_status(TaskStatus status) {
    status_ = status;
    touch();
}

void TaskNode::set_priority(TaskPriority priority) {
    priority_ = priority;
    touch();
}

void TaskNode::add_child(const Ptr& child) {
    if (!child) {
        throw std::invalid_argument("TaskNode::add_child: child is null");
    }

    child->parent_ = this;
    children_.push_back(child);
    touch();
}

bool TaskNode::remove_child_by_id(const std::string& id) {
    const auto it = std::remove_if(
        children_.begin(),
        children_.end(),
        [&](const Ptr& c) { return c && c->get_id() == id; }
    );

    if (it == children_.end()) {
        return false;
    }

    for (auto jt = it; jt != children_.end(); ++jt) {
        if (*jt) {
            (*jt)->parent_ = nullptr;
        }
    }

    children_.erase(it, children_.end());
    touch();
    return true;
}

TaskNode::Ptr TaskNode::find_child_by_id(const std::string& id) const {
    for (const auto& c : children_) {
        if (c && c->get_id() == id) {
            return c;
        }
    }
    return nullptr;
}

TaskNode::Ptr TaskNode::find_child_by_title(std::string_view title) const {
    for (const auto& c : children_) {
        if (c && c->get_title() == title) {
            return c;
        }
    }
    return nullptr;
}

std::string TaskNode::get_path() const {
    // Virtual workspace root should have title "/" and parent_ == nullptr.
    if (!parent_) {
        return "/";
    }

    std::string out;
    const TaskNode* cur = this;

    // Walk up using raw parent pointers; this is safe as long as nodes live
    // for the lifetime of the tree (which they do via shared_ptr ownership).
    while (cur && cur->parent_) {
        out.insert(0, "/" + cur->title_);
        cur = cur->parent_;
    }

    if (out.empty()) {
        return "/";
    }
    return out + "/";
}

void TaskNode::touch() { updated_at_ = std::time(nullptr); }

std::string TaskNode::generate_id() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    static thread_local std::uniform_int_distribution<int> dist(0, 15);

    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << dist(rng);
    }
    return oss.str();
}

TaskNode::Ptr resolve_path(
    const TaskNode::Ptr& workspace_root,
    std::string_view absolute_path
) {
    if (!workspace_root) {
        return nullptr;
    }

    if (absolute_path.empty() || absolute_path == "/") {
        return workspace_root;
    }

    auto current = workspace_root;
    for (const auto seg : split_path(absolute_path)) {
        auto next = current->find_child_by_title(seg);
        if (!next) {
            return nullptr;
        }
        current = next;
    }

    return current;
}