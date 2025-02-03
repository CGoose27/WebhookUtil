#!/bin/bash

build_and_clean() {
  cmake -DCMAKE_BUILD_TYPE=Release "$1" .
  make clean
  make
}

build_and_clean "-DCMAKE_C_FLAGS=-m32"
build_and_clean "-DCMAKE_C_FLAGS=-m64"

build_and_clean "-DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ -G 'MinGW Makefiles'"
build_and_clean "-DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -G 'MinGW Makefiles'"
