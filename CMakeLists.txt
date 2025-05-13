cmake_minimum_required(VERSION 2.29)

project(malphas-backend)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)

# This keeps Clangd from getting confused over imports when developing in Vim
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# --- DAO Code Generator ---
if (NOT DEFINED NOCODEGEN)
	add_executable(gen_dao codegen/gen.cpp src/cfg/Config.cpp src/parser.cpp src/util.cpp)

	add_custom_command(TARGET gen_dao POST_BUILD
        	COMMAND mkdir -p include/dao
        	COMMAND gen_dao > include/dao/dao.h
        	COMMENT "Create the DAO directory and invoke the DAO generator")
endif()

cmake_policy(SET CMP0167 NEW)

# --- Malphas Backend ---
add_executable(malphas src/Main.cpp src/Database.cpp src/cfg/Config.cpp src/util.cpp src/parser.cpp
        src/middleware/AuthFilter.cpp src/Api.cpp vendor/Bcrypt.cpp/src/bcrypt.cpp
        vendor/Bcrypt.cpp/src/blowfish.cpp)

target_compile_definitions(malphas PUBLIC __CMAKE_BUILD__)

# --- For Crow SSL support ---
find_package(OpenSSL REQUIRED)

# --- The boost library, used for generating UUIDs ---
find_package(Boost REQUIRED)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find the Boost library")
endif ()

target_include_directories(malphas PRIVATE ${Boost_INCLUDE_DIRS})

# Make the crow library directly #include-able
target_include_directories(malphas PRIVATE vendor/crow vendor/Bcrypt.cpp/include)

if (NOT DEFINED NOCODEGEN)
	target_include_directories(gen_dao PRIVATE vendor/crow)
endif()

# .. do the same for the headers in `include/`
target_include_directories(malphas PRIVATE include)

# This provides the database connection utilities
find_package(PostgreSQL REQUIRED)
target_include_directories(malphas PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(malphas PRIVATE include/dao)

if (NOT DEFINED NOCODEGEN)
	target_include_directories(gen_dao PRIVATE ${PostgreSQL_INCLUDE_DIRS})
	target_include_directories(gen_dao PRIVATE include)
endif()

# Locate the ASIO dev headers (only on UNIX systems!)
if (UNIX)
    find_path(ASIO_INCLUDE_DIR asio.hpp)
    if (ASIO_INCLUDE_DIR)
        message("Successfully located the ASIO library.\n")
        target_include_directories(malphas PRIVATE ${ASIO_INCLUDE_DIR})
	if (NOT DEFINED NOCODEGEN)
        	target_include_directories(gen_dao PRIVATE ${ASIO_INCLUDE_DIR})
	endif()
    else ()
        message(FATAL_ERROR "Could not find the ASIO library")
    endif ()
endif ()

target_link_libraries(malphas PRIVATE ${PostgreSQL_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto ${Boost_LIBRARIES})

if (NOT DEFINED NOCODEGEN)
	target_link_libraries(gen_dao PRIVATE ${PostgreSQL_LIBRARIES})
	add_dependencies(malphas gen_dao)
endif ()
