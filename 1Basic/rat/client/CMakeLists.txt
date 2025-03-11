cmake_minimum_required(VERSION 3.12)

project(rat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

# find_package(unofficial-cunit CONFIG REQUIRED)

link_directories(build)

add_library(CommandManager src/CommandManager.cpp)
target_include_directories(CommandManager PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(ClientSocket src/ClientSocket.cpp)
target_include_directories(ClientSocket PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Client src/Client.cpp)
target_link_libraries(Client PRIVATE CommandManager)
target_link_libraries(Client PRIVATE ClientSocket)
target_include_directories(Client PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_executable(rat src/main.cpp)
target_link_libraries(rat PRIVATE Client)
target_include_directories(rat PRIVATE "${PROJECT_SOURCE_DIR}/include")

# add_executable(testCommandManager test/testCommandManager)
# target_link_libraries(testCommandManager PRIVATE unofficial::cunit::cunit)
# target_link_libraries(testCommandManager PRIVATE CommandManager)
# target_include_directories(testCommandManager PRIVATE "${PROJECT_SOURCE_DIR}/include")