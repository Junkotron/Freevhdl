#!/bin/bash

# Make verilog sim

yosys -p "\
  read_verilog test.v bb.v; \
  hierarchy -top top; \
  proc;\
  write_cxxrtl -noflatten -header gen_design.cpp\
"


clang++ -g -std=c++11 \
  -I $(yosys-config --datdir)/include/backends/cxxrtl/runtime \
  -I . \
  gen_design.cpp bb_impl.cpp sim_main.cpp \
  -o sim_executable

# Make vhdl sim

# Compile the verilog module into rtlil
yosys -p "read_verilog bb.v; write_rtlil bb.rtlil"

# Prepare the "header" for the test.vhd file in work-obj93.cf
ghdl -a test.vhd

# Do the same with the overlaying vhdl module using component inferal
yosys -m ghdl -p "ghdl my_bb_vhdl; write_rtlil test_vhdl.rtlil"

# combine to a cxxrtl simulation
yosys -p " \
  read_rtlil test_vhdl.rtlil bb.rtlil; \
  hierarchy -top my_bb_vhdl; \
  proc; \
  write_cxxrtl -noflatten -header gen_design_vhdl.cpp \
"

clang++ -g -std=c++11 \
  -I $(yosys-config --datdir)/include/backends/cxxrtl/runtime \
  -I . \
  gen_design_vhdl.cpp bb_impl.cpp sim_main_vhdl.cpp \
  -o sim_vhdl_executable
