cmake_minimum_required(VERSION 3.12)

project(ratServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# find_package(unofficial-cunit CONFIG REQUIRED)

link_directories(build)

add_library(EventLogger src/EventLogger.cpp)
target_include_directories(EventLogger PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(ServerSocket src/ServerSocket.cpp)
target_link_libraries(ServerSocket PRIVATE EventLogger)
target_include_directories(ServerSocket PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Server src/Server.cpp)
target_link_libraries(Server PRIVATE ServerSocket)
target_include_directories(Server PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_executable(ratServer src/main.cpp)
target_link_libraries(ratServer PRIVATE Server)
target_include_directories(ratServer PRIVATE "${PROJECT_SOURCE_DIR}/include")

# add_executable(testCommandManager test/testCommandManager)
# target_link_libraries(testCommandManager PRIVATE unofficial::cunit::cunit)
# target_link_libraries(testCommandManager PRIVATE CommandManager)
# target_include_directories(testCommandManager PRIVATE "${PROJECT_SOURCE_DIR}/include")