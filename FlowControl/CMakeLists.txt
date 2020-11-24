cmake_minimum_required(VERSION 3.16.3)
project(FLOWCONTROL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wextra")

add_library(externalLib SHARED IMPORTED)
set_target_properties(externalLib PROPERTIES IMPORTED_LOCATION /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/include/library.so)


add_executable(
    sendercontroller
    sendercontroller.cpp
)

add_executable(
    receivercontroller
    receivercontroller.cpp
)

add_executable(
    channel
    channel.cpp
)

target_link_libraries(receivercontroller pthread rt)
target_link_libraries(channel pthread )
target_link_libraries(sendercontroller externalLib pthread rt)