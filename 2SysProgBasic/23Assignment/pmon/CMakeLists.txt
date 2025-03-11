cmake_minimum_required(VERSION 3.12)

project(pmon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

link_directories(build)

add_library(EventLogger src/EventLogger.cpp)
target_include_directories(EventLogger PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Utils src/Utils.cpp)
target_include_directories(Utils PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(FastSystem src/FastSystem.cpp)
target_link_libraries(FastSystem PRIVATE EventLogger)
target_link_libraries(FastSystem PRIVATE Utils)
target_include_directories(FastSystem PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Process src/Process.cpp)
target_link_libraries(Process PRIVATE EventLogger)
target_include_directories(Process PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Configuration src/Configuration.cpp)
target_link_libraries(Configuration PRIVATE EventLogger)
target_include_directories(Configuration PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(Event src/Event.cpp)
target_include_directories(Event PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(OverloadEvent src/OverloadEvent.cpp)
target_link_libraries(OverloadEvent PRIVATE Event)
target_link_libraries(OverloadEvent PRIVATE EventLogger)
target_link_libraries(OverloadEvent PRIVATE Process)
target_link_libraries(OverloadEvent PRIVATE Configuration)
target_include_directories(OverloadEvent PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(PmonController src/PmonController.cpp)
target_link_libraries(PmonController PRIVATE FastSystem)
target_link_libraries(PmonController PRIVATE Process)
target_link_libraries(PmonController PRIVATE Configuration)
target_link_libraries(PmonController PRIVATE OverloadEvent)
target_include_directories(PmonController PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_executable(pmon src/pmon.cpp)
target_link_libraries(pmon PRIVATE PmonController)
target_include_directories(pmon PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_library(PmonUIController src/PmonUIController.cpp)
target_link_libraries(PmonUIController PRIVATE FastSystem)
target_link_libraries(PmonUIController PRIVATE EventLogger)
target_link_libraries(PmonUIController PRIVATE Configuration)
target_include_directories(PmonUIController PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_executable(pmonui src/pmonui.cpp)
target_link_libraries(pmonui PRIVATE PmonUIController)
target_include_directories(pmonui PRIVATE "${PROJECT_SOURCE_DIR}/include")