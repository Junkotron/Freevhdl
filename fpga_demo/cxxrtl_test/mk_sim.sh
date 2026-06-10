#!/bin/bash

yosys -p "read_verilog blinken.v; write_cxxrtl blinken.cpp"

clang++ -g -O3 -std=c++14 -I `yosys-config --datdir`/include/backends/cxxrtl/runtime  main.cpp -o tb
