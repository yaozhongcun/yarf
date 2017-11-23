#-----------------------------
# proto conf
#-----------------------------
cmake_minimum_required (VERSION 2.6)


file(GLOB_RECURSE SOURCES
    ./*.cc)


add_library(${PROJECT_NAME} ${SOURCES})


