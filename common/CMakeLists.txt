project(SockitCommon)

set(CMAKE_CXX_STANDARD 11)

add_library(SockitCommon INTERFACE packets.hpp)
target_include_directories(SockitCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(SockitCommon PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(SockitCommon INTERFACE yojimbo)
