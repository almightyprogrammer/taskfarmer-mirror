#ifndef TASKFARMER_V2_RBAC_HPP
#define TASKFARMER_V2_RBAC_HPP

#include <string>
#include <vector>
#include <optional>

enum class Role {
    SUPERUSER,
    MANAGER,
    ADMIN,
    DATA_ADMIN,
    USER
};

enum class Permission {
    TASK_READ,
    TASK_CREATE,
    TASK_MODIFY,
    TASK_DELETE,
    ADMIN_DB,
    ADMIN_USERS,
};

struct CallerContext {
    std::string user_id;
    std::vector<Role> roles;
};

std::optional<Role> role_from_string(std::string_view role_string);
std::optional<std::string> role_to_string(const Role& role);



#endif