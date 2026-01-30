#include "../include/Authoriser.hpp"

bool Authoriser::can(const CallerContext& caller, Permission perm) const {
    for (const auto role : caller.roles) {
        if (role_grants(role, perm)) {
            return true;
        }
    }
    return false;
}

bool Authoriser::role_grants(Role role, Permission perm) {
    if (role == Role::SUPERUSER) {
        return true;
    }

    switch (role) {
        case Role::MANAGER: {
            switch (perm) {
                case Permission::TASK_READ:
                case Permission::TASK_CREATE:
                case Permission::TASK_MODIFY:
                case Permission::TASK_DELETE:
                    return true;

                case Permission::ADMIN_DB:
                case Permission::ADMIN_USERS:
                    return false;
            }
            return false;
        }

        case Role::USER: {
            switch (perm) {
                case Permission::TASK_READ:
                case Permission::TASK_MODIFY:
                    return true;

                case Permission::TASK_CREATE:
                case Permission::TASK_DELETE:
                case Permission::ADMIN_DB:
                case Permission::ADMIN_USERS:
                    return false;
            }
            return false;
        }

        case Role::DATA_ADMIN: {
            switch (perm) {
                case Permission::TASK_READ:
                case Permission::ADMIN_DB:
                    return true;

                case Permission::TASK_CREATE:
                case Permission::TASK_MODIFY:
                case Permission::TASK_DELETE:
                case Permission::ADMIN_USERS:
                    return false;
            }
            return false;
        }
    }

    return false;
}