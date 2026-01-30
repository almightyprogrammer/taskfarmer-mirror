#ifndef TASKFARMER_V2_AUTHORISER_HPP
#define TASKFARMER_V2_AUTHORISER_HPP

#include "Rbac.hpp"

class Authoriser {
public:
    bool can(const CallerContext& caller, Permission perm) const;

private:
    static bool role_grants(Role role, Permission perm);
};


#endif