#
cmake_minimum_required(VERSION 2.6)


set(EXECUTABLE_OUTPUT_PATH ${WORK_DIR}/bin/test)

aux_source_directory(. SRC)


foreach (test ${SRC} )
  string(REPLACE ".cc" "" target ${test})
  string(REPLACE "./test_" "" target ${target})
  message(${target})
  add_executable(${target} ${test})
  target_link_libraries(${target} yarf gtest_main gtest pthread) 
endforeach(test)

