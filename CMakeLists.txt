cmake_minimum_required (VERSION 3.10)

project (MpdWS)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package (docopt COMPONENTS CXX REQUIRED)
find_package (LibMPDClient)
find_package (websocketpp)

include_directories(${DOCOPT_INCLUDE_DIRS})
include_directories (${WEBSOCKETPP_ROOT} ${WEBSOCKETPP_INCLUDE})
include_directories (${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${LIBMPDCLIENT_INCLUDE_DIRS})

set (WEBSOCKETPP_PLATFORM_LIBS pthread rt)
add_definitions (${WEBSOCKETPP_DEFINITIONS})

add_subdirectory (include)
add_executable (mpdws main.cpp)


target_link_libraries (mpdws PUBLIC docopt ${LIBMPDCLIENT_LIBRARY} ${WEBSOCKETPP_LIBRARIES} Json)
