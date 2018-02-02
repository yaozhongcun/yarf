#!/usr/bin/python
# -*- coding: utf-8 -*-

import os,re,sys
import platform

from yarf_util import *


#############################################################
# description
svr_dir_desc="""
An application code is placed in directory that is equal to yarf like this:
--your_workspace_dir

----yarf
------include
------lib
------3rd
------tools

----your_svr_name
------CMakeLists.txt
------build
------include
------src
--------your_svr_name_main.cc
------bin
------conf
"""

svr_name_desc="""Please input your svr name: 
"""

svr_type_desc="""Please input your svr type: 
1 simple app
2 multi thread proxy
3 async process svr
4 net svr
"""

svr_include_desc="""Please input your include type: 
1 simple, only log.h and err.h, i will add by myself
2 full, all yarf include files
"""


#############################################################
# code
cmake_main="""
########################################################
#
cmake_minimum_required (VERSION 2.8)

project($$svr_name$$)


#--------------------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC -std=c++0x")

#--------------------------------------------------------
# child dir can not use CMAKE_CURRENT_SOURCE_DIR
set(WORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)

set(YARF_INC_DIR ${WORK_DIR}/../yarf/include/)
set(YARF_LIB_DIR ${WORK_DIR}/../yarf/lib)

set(GTEST_INC_DIR ${WORK_DIR}/../yarf/3rd/gtest1.8)
set(GTEST_LIB_DIR ${WORK_DIR}/../yarf/lib)

set(SELF_INC_DIR ${WORK_DIR}/include/)


#--------------------------------------------------------
include_directories(${YARF_INC_DIR} ${GTEST_INC_DIR} ${SELF_INC_DIR})
link_directories(${YARF_LIB_DIR})

set(LIBRARY_OUTPUT_PATH ${WORK_DIR}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${WORK_DIR}/bin)

add_subdirectory(src)
"""

cmake_src="""
cmake_minimum_required(VERSION 2.8)

aux_source_directory(. SRC)

add_executable(${PROJECT_NAME} ${SRC}) 

target_link_libraries(${PROJECT_NAME} yarf)
#R7core comm proto_gen table protobuf flatbuffers pthread mysqlclient dl rt)     
"""
#R7core comm proto_gen table protobuf flatbuffers pthread mysqlclient dl rt) 

main_inc_begin="""
#ifndef $$MACRO$$
#define $$MACRO$$

#include "yarf/log.h"
#include "yarf/err.h"
"""
main_inc_end="""
#endif // $$MACRO$$

"""
simple_main_cc="""

#include "$$svr_name$$.h"

int main() {
  DEBUG_LOG("main begin");
  return 0;
}

"""

#
def main():
    #print(os.path.split(sys.argv[0])[0])
    #input 
    print(svr_dir_desc);
    svr_name=raw_input(svr_name_desc)
    svr_type=raw_input(svr_type_desc);
    inc_type=raw_input(svr_include_desc);

    #dir
    main_dir=os.path.split(sys.argv[0])[0] + "/../../"+svr_name
    mkdir_wrapper(main_dir)

    mkdir_wrapper(main_dir+"/build")

    svr_inc_dir=main_dir+"/include"
    mkdir_wrapper(svr_inc_dir)

    svr_src_dir=main_dir+"/src"
    mkdir_wrapper(svr_src_dir)

    mkdir_wrapper(main_dir+"/bin")

    mkdir_wrapper(main_dir+"/conf")

    mkdir_wrapper(main_dir+"/3rd")

    #code
    try:
        inc_file = open(svr_inc_dir+"/"+svr_name+".h", 'w')
        src_file = open(svr_src_dir+"/"+svr_name+"_main.cc", 'w')
        main_make_file = open(main_dir+"/CMakeLists.txt", 'w')
        src_make_file = open(svr_src_dir+"/CMakeLists.txt", 'w')
    except Exception as err:
        print err
        return

    main_make_file.write(cmake_main.replace("$$svr_name$$", svr_name));
    src_make_file.write(cmake_src.replace("$$svr_name$$", svr_name));
    src_file.write(simple_main_cc.replace("$$svr_name$$", svr_name));

    macro=svr_name.upper() + "_" + svr_name.upper() + "_H_"
    inc_file.write(main_inc_begin.replace("$$MACRO$$", macro));
    inc_file.write(main_inc_end.replace("$$MACRO$$", macro));

main()

