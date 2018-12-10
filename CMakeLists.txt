#-----------------------------
# proto conf
#-----------------------------
cmake_minimum_required (VERSION 2.6)

project(yarf)


########################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC -std=c++0x -DMAC")

########################################################
# child dir can not use CMAKE_CURRENT_SOURCE_DIR
set(WORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)

set(YARF_INC_DIR ${WORK_DIR}/include/)
set(YARF_LIB_DIR ${WORK_DIR}/lib)

set(GTEST_INC_DIR ${WORK_DIR}/../3rd/gtest1.8/include)
set(GTEST_LIB_DIR ${WORK_DIR}/../3rd/gtest1.8/lib)

set(PROTO_INC_DIR ${WORK_DIR}/../3rd/protobuf3.3/include)
set(PROTO_LIB_DIR ${WORK_DIR}/../3rd/protobuf3.3/lib)

########################################################
include_directories(${YARF_INC_DIR} ${GTEST_INC_DIR} ${PROTO_INC_DIR})
link_directories(${YARF_LIB_DIR} ${GTEST_LIB_DIR} ${PROTO_LIB_DIR})

set(LIBRARY_OUTPUT_PATH ${YARF_LIB_DIR}/)
set(EXECUTABLE_OUTPUT_PATH ${WORK_DIR}/bin)

add_subdirectory(src)
