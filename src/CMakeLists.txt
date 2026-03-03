if (NOT MSVC)
    add_compile_options(-O2 -Wall -Werror -Wextra -Wno-unused-variable)
    if (WIN32)
        add_compile_options(-static -static-libgcc -static-libstdc++)
    endif()
else()
    message(FATAL_ERROR "Compiling with MSVC is not supported")
endif()

include_directories(../lib/mizuna/include)
include_directories(../lib/mizuna/lib/hk/include)
include_directories(../lib)
include_directories(.)

add_executable(mizuna-utils)
add_executable(al-search)
add_executable(al-config)

find_library(ZSTD_LIBRARY NAMES zstd lzstd libzstd)
target_link_libraries(mizuna-utils PRIVATE ${ZSTD_LIBRARY})

target_sources(mizuna-utils
    PRIVATE
        mizuna-utils.cpp
)

target_sources(al-search
    PRIVATE
        al-search.cpp
        config.cpp
)

target_sources(al-config
    PRIVATE
        al-config.cpp
        config.cpp
)

target_link_libraries(mizuna-utils PRIVATE mizuna)
target_link_libraries(al-search PRIVATE mizuna)
target_link_libraries(al-config PRIVATE mizuna)