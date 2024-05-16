cmake_minimum_required(VERSION "3.25.0")

project(service_registry_tests)
set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIRS thirdparty ../include)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

add_executable(service_registry_tests ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(service_registry_tests PRIVATE service_registry)

set_property(TARGET service_registry_tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")