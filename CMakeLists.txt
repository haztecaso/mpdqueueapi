cmake_minimum_required(VERSION 3.10)

project(MpdHtmlStats)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package(LibMPDClient)
find_package(Boost)
find_package(websocketpp)

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}	${LIBMPDCLIENT_INCLUDE_DIR})


add_subdirectory(include)
add_executable(mpdqueueapi main.cpp)

target_link_libraries(mpdqueueapi ${LIBMPDCLIENT_LIBRARY} Json)
