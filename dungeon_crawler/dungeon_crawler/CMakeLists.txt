cmake_minimum_required(VERSION 3.10)
project(DungeonCrawler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict warnings to demonstrate code quality
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include_directories(include)

add_executable(dungeon_crawler
    src/main.cpp
    src/map.cpp
    src/entities.cpp
    src/renderer.cpp
    src/input.cpp
    src/highscore.cpp
)

# Install target (optional)
install(TARGETS dungeon_crawler DESTINATION bin)
