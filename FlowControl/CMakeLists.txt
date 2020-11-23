cmake_minimum_required(VERSION 3.16.3)
project(FLOWCONTROL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wextra")

add_executable(
    sendercontroller
    sendercontroller.cpp
)

add_executable(
    receivercontroller
    receivercontroller.cpp
)

target_link_libraries(receivercontroller pthread)
target_link_libraries(receivercontroller rt)
target_link_libraries(sendercontroller pthread)
target_link_libraries(sendercontroller rt)