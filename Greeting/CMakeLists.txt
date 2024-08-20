cmake_minimum_required(VERSION 3.22.1)

set(PROJECT_NAME Greeting)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wpedantic)

project(${PROJECT_NAME} VERSION 1.0
                        DESCRIPTION "CMake Greating"
                        HOMEPAGE_URL "www.project.ulr"
                        LANGUAGES CXX)

add_subdirectory(repo)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE Greeting)
target_include_directories(test PRIVATE repo/Greeting)

add_executable(test test/test.cpp)
target_link_libraries(test_led PRIVATE led repo)
target_include_directories(test_led PRIVATE repo/Greeting include)