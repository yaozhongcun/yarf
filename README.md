# yarf
yet another rpc framework

compile env:

  os: CentOS release 6.9 (Final)  2.6.32-696.el6.x86_64

  gcc: gcc (GCC) 7.2.0

  cmake: cmake version 2.8.12.2

mac:
  modify CMakeList.txt, add -DMAC

3rd lib: 

  gtest 1.8, googletest-release-1.8.0.tar.gz
  
  tar
  
  mkdir build
  
  cmake .. & make
  
  copy include & lib to dir:
  
    include: 3rd/gtest1.8/gtest/gtest.h
    lib: lib/libtest.a lib/libtest_main.a 

  protobuf3.3, protobuf-cpp-3.3.0.tar.gz
  
  tar
   
  ./configure --prefix=your_path
  
  make & make install
  
  copy include & lib to dir:
  
    include: 3rd/protobuf3.3/google/...
    lib:  lib/libprotobuf.a

