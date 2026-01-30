#ifndef TASKFARMER_V2_USERSERVICE_HPP
#define TASKFARMER_V2_USERSERVICE_HPP

#include "Database.hpp"
#include "User.hpp"
#include <unordered_map>


class UserService {
public:
    explicit UserService(Database& db);

    void init();

    bool grant_role(std::string_view user_id, std::string_view role_string);

    bool create_user(std::string_view name);

    bool create_user(std::string_view name, std::string_view role_string);

private:
    Database& db_;
    std::unordered_map<std::string_view, User> user_map_;


};


#endif