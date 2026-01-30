#ifndef TASKFARMER_V2_USER_HPP
#define TASKFARMER_V2_USER_HPP

#include <random>
#include <iostream>
#include <sstream>

#include "Rbac.hpp"

struct User {
    std::string id;
    std::string name;
    std::vector<Role> roles;
};

inline std::string generate_uuid() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    static thread_local std::uniform_int_distribution<int> dist(0, 15);

    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << dist(rng);
    }
    return oss.str();
}

#endif