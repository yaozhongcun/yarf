#-----------------------------
# proto conf
#-----------------------------
cmake_minimum_required (VERSION 2.6)


file(GLOB_RECURSE SOURCES
    ./*.cc)


include_directories(${YARF_INC_DIR} ${PROTO_INC_DIR})


add_library(${PROJECT_NAME} ${SOURCES})





