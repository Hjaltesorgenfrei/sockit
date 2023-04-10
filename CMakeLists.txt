cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(SockIT)

# ---- Dependencies ----
include(cmake/CPM.cmake)
add_subdirectory(yojimbo)

# ---- Executable ----

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
