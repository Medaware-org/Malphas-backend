cmake_minimum_required(VERSION 2.29)

project(malphas)

set(CMAKE_CXX_STANDARD 14)

find_path(ASIO_INCLUDE_DIR asio.hpp)

if (ASIO_INCLUDE_DIR)
    message("Successfully located the ASIO library.\n")
    include_directories(vendor/crow ${ASIO_INCLUDE_DIR})
    add_executable(malphas-backend src/Main.cpp)
else ()
    message(FATAL_ERROR "Could not find the ASIO library")
endif ()
