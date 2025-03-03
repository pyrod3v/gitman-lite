# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Gitman LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required libraries
find_package(CURL REQUIRED)

# Add executable
add_executable(gm
    ${CMAKE_SOURCE_DIR}/src/gitman.c
    ${CMAKE_SOURCE_DIR}/src/main.c
)

# Include directories
target_include_directories(gm PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link libraries
target_link_libraries(gm PRIVATE CURL::libcurl)