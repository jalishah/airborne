#!/bin/sh
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/cortex_a8.cmake .. && make
