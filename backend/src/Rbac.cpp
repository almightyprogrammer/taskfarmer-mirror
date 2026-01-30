#include "../include/Rbac.hpp"

std::optional<Role> role_from_string(std::string_view role_string) {
    if (role_string == "SUPERUSER") return Role::SUPERUSER;
    if (role_string == "MANAGER") return Role::MANAGER;
    if (role_string == "ADMIN") return Role::ADMIN;
    if (role_string == "DATA_ADMIN") return Role::DATA_ADMIN;
    if (role_string == "USER") return Role::USER;
    return std::nullopt;
}


std::optional<std::string> role_to_string(const Role& role) {
    switch (role) {
        case Role::SUPERUSER:
            return "SUPERUSER";
        case Role::MANAGER:
            return "MANAGER";
        case Role::ADMIN:
            return "ADMIN";
        case Role::DATA_ADMIN:
            return "DATA_ADMIN";
        case Role::USER:
            return "USER";
    }
}


