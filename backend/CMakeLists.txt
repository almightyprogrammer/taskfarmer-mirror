cmake_minimum_required(VERSION 3.28.3)
project(taskfarmer_v2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(SQLite3 REQUIRED)

add_library(taskfarmer_core
    src/TaskNode.cpp
    src/Database.cpp
    src/HttpServer.cpp
    src/TaskService.cpp
    src/Rbac.cpp
    src/UserService.cpp
    src/Authoriser.cpp
)

target_include_directories(taskfarmer_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskfarmer_core PUBLIC
    httplib::httplib
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
)

add_executable(taskfarmer_v2
    main.cpp
)

target_link_libraries(taskfarmer_v2 PRIVATE
    taskfarmer_core
)