#include "../include/UserService.hpp"


UserService::UserService(Database& db) : db_(db) {
    init();
}

void UserService::init() {
    db_.open();
    db_.init_schema();
}

bool UserService::grant_role(
    std::string_view user_id,
    std::string_view role_string
) {
    // checl that role_string is valid
    std::optional<Role> role = role_from_string(role_string);
    if (role.has_value()) {
        return db_.insert_user_role(user_id, role_string);
    } else {
        return false;
    }
}

bool UserService::create_user(std::string_view name) {
    std::string user_id = generate_uuid();

    if (db_.insert_user(user_id, name)) {
        return true;
    } else {
        return false;
    }
}

bool UserService::create_user(std::string_view name, std::string_view role_string) {
    std::string user_id = generate_uuid();

    if (db_.insert_user(user_id, name)) {
        if (grant_role(user_id, role_string)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}
