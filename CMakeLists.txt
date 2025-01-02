cmake_minimum_required(VERSION 2.29)

project(malphas-backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

# This keeps Clangd from getting confused over imports when developing in Vim
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# --- DAO Code Generator ---
add_executable(gen_dao codegen/gen.cpp src/cfg/Config.cpp src/parser.cpp src/util.cpp)

add_custom_command(TARGET gen_dao POST_BUILD
        COMMAND mkdir -p include/dao
        COMMAND gen_dao > include/dao/dao.h
        COMMENT "Create the DAO directory and invoke the DAO generator")

# --- Malphas Backend ---
add_executable(malphas src/Main.cpp src/Database.cpp src/cfg/Config.cpp src/util.cpp src/parser.cpp src/middleware/AuthFilter.cpp src/Api.cpp)

# Make the crow library directly #include-able
target_include_directories(malphas PRIVATE vendor/crow)
target_include_directories(gen_dao PRIVATE vendor/crow)

# .. do the same for the headers in `include/`
target_include_directories(malphas PRIVATE include)

# This provides the database connection utilities
find_package(PostgreSQL REQUIRED)
target_include_directories(malphas PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(malphas PRIVATE include/dao)
target_include_directories(gen_dao PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(gen_dao PRIVATE include)

# Locate the ASIO dev headers (only on UNIX systems!)
if (UNIX)
    find_path(ASIO_INCLUDE_DIR asio.hpp)
    if (ASIO_INCLUDE_DIR)
        message("Successfully located the ASIO library.\n")
        target_include_directories(malphas PRIVATE ${ASIO_INCLUDE_DIR})
        target_include_directories(gen_dao PRIVATE ${ASIO_INCLUDE_DIR})
    else ()
        message(FATAL_ERROR "Could not find the ASIO library")
    endif ()
endif ()

target_link_libraries(malphas PRIVATE ${PostgreSQL_LIBRARIES})
target_link_libraries(gen_dao PRIVATE ${PostgreSQL_LIBRARIES})

add_dependencies(malphas gen_dao)