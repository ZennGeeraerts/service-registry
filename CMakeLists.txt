cmake_minimum_required(VERSION "3.25.0")

project(service_registry)
set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIRS include)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

add_definitions(-DUNICODE)

add_subdirectory(tests)