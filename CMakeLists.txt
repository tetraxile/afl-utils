cmake_minimum_required(VERSION 3.15)

project(afl-utils)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-O2 -Wall -Werror -Wextra -Wno-unused-variable)
    if (WIN32)
        add_compile_options(-static -static-libgcc -static-libstdc++)
    endif()
endif()

add_subdirectory(lib/afl)

include_directories(lib/afl/include)

set(MAIN_SOURCES src/main.cpp)
set(COPY_SOURCES src/copy.cpp)
set(SEARCH_SOURCES src/search.cpp)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})
add_executable(afl-search ${SEARCH_SOURCES})
add_executable(afl-copy ${COPY_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE afl)
target_link_libraries(afl-search PRIVATE afl)
target_link_libraries(afl-copy PRIVATE afl)
