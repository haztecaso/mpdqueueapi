cmake_minimum_required (VERSION 3.10)

project (MpdHtmlStats)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package (LibMPDClient)
find_package (websocketpp)

set (WEBSOCKETPP_PLATFORM_LIBS pthread rt)
add_definitions (${WEBSOCKETPP_DEFINITIONS})
include_directories (${WEBSOCKETPP_ROOT} ${WEBSOCKETPP_INCLUDE})

include_directories (${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${LIBMPDCLIENT_INCLUDE_DIRS})

add_subdirectory (include)
add_executable (mpdqueueapi main.cpp)

target_link_libraries (mpdqueueapi PUBLIC ${LIBMPDCLIENT_LIBRARY} ${WEBSOCKETPP_LIBRARIES} Json)
