if (NOT MSVC)
    add_compile_options(-O2 -Wall -Werror -Wextra -Wno-unused-variable)
    if (WIN32)
        add_compile_options(-static -static-libgcc -static-libstdc++)
    endif()
else()
    message(FATAL_ERROR "Compiling with MSVC is not supported")
endif()

include_directories(../lib/afl/include)
include_directories(../lib/afl/lib/hk/include)
include_directories(../lib)
include_directories(.)

add_executable(afl-utils)
add_executable(afl-search)
add_executable(afl-config)

target_sources(afl-utils
    PRIVATE
        afl-utils.cpp
)

target_sources(afl-search
    PRIVATE
        afl-search.cpp
        config.cpp
)

target_sources(afl-config
    PRIVATE
        afl-config.cpp
        config.cpp
)

target_link_libraries(afl-utils PRIVATE afl)
target_link_libraries(afl-search PRIVATE afl)
target_link_libraries(afl-config PRIVATE afl)