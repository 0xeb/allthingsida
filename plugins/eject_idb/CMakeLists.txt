cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(eject_idb)

add_definitions(-DUNICODE -D_UNICODE)

# eject IDB
set(CMAKE_CXX_STANDARD 17)

add_executable(eject_idb eject_idb.cpp utils.hpp)

include($ENV{IDASDK}/ida-cmake/common.cmake)

set(PLUGIN_NAME              eject_plugin)
set(PLUGIN_SOURCES           eject_plugin.cpp utils.hpp)
set(PLUGIN_RUN_ARGS          "-t -z10000") # Debug messages for the debugger

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

generate()
disable_ida_warnings(eject_plugin)
