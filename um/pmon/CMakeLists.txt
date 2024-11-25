cmake_minimum_required(VERSION 3.12)

project(pmon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

link_directories(build)

add_executable(pmon pmon/src/main.cpp)

add_executable(pmonc pmonc/src/main.cpp)