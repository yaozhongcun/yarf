#-----------------------------
# proto conf
#-----------------------------
cmake_minimum_required (VERSION 2.6)

project(yarf)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC -std=c++0x")

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(WORKSPACE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(YARF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(YARF_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)


add_subdirectory(src)
