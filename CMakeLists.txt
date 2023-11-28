cmake_minimum_required(VERSION 3.9)
project(Serializer)

set(CMAKE_CXX_STANDARD 17)

# Global Configure
set(GLOBAL_INCLUDE_DIR 
"Vendor"
"Dependencies/clang/include"
)
set(GLOBAL_LINK_DIR 
"Dependencies/clang/lib"
)
set(GLOBAL_LIB "libclang" )
file(GLOB_RECURSE GLOBAL_FILE
"src/*.cpp" 
"src/*.h"
)


# compile mode
#set(CMAKE_BUILD_TYPE "Debug")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # Debug Configure
    add_definitions(-DDEBUG)
    add_definitions(-DENABLE_DEBUG_LOG)
    set(DEBUG_INCLUDE_DIR)
    set(DEBUG_LINK_DIR )
    set(DEBUG_LIB)
    set(DEBUG_FILE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    # Release Configure
    add_definitions(-DNDEBUG)
    set(RELEASE_INCLUDE_DIR)
    set(RELEASE_LINK_DIR )
    set(RELEASE_LIB)
    set(RELEASE_FILE)
else ()
    # Other Configure
endif ()
include("vs_config.cmake")
set(PROJECT_FILE ${GLOBAL_FILE} ${DEBUG_FILE} ${RELEASE_FILE})
set(PROJECT_INCLUDE_DIR ${GLOBAL_INCLUDE_DIR} ${DEBUG_INCLUDE_DIR} ${RELEASE_INCLUDE_DIR})
set(PROJECT_LINK_DIR ${GLOBAL_LINK_DIR} ${DEBUG_LINK_DIR} ${RELEASE_LINK_DIR})
set(PROJECT_LIB ${GLOBAL_LIB} ${DEBUG_LIB} ${RELEASE_LIB})
link_directories(${PROJECT_LINK_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
add_executable(${PROJECT_NAME}  ${PROJECT_FILE})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB})
