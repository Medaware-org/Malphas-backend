cmake_minimum_required(VERSION 2.29)

project(malphas)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

# Compiled binaries are to be put in the 'build' directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

# This keeps Clangd from getting confused over imports when developing in Vim
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Make the crow library directly #include-able
include_directories(vendor/crow)

# .. do the same for the headers in `include/`
include_directories(include)

# This provides the database connection utilities
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})

# Locate the ASIO dev headers (only on UNIX systems!)
if (UNIX)
    find_path(ASIO_INCLUDE_DIR asio.hpp)
    if (ASIO_INCLUDE_DIR)
        message("Successfully located the ASIO library.\n")
        include_directories(malphas ${ASIO_INCLUDE_DIR})
    else ()
        message(FATAL_ERROR "Could not find the ASIO library")
    endif ()
endif ()

add_executable(malphas src/Main.cpp src/Database.cpp src/cfg/Config.cpp)
target_link_libraries(malphas ${PostgreSQL_LIBRARIES})
