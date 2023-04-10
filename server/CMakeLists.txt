project(SockitServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(SockitServer main.cpp)
target_link_libraries(SockitServer asio SockitCommon yojimbo)