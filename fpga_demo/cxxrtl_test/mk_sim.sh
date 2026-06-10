#!/bin/bash


# Compound of verilog and vhdl verilog ands a vector together
# and then in vhdl it is inverted to produce a nand gate

yosys -p "read_verilog vector.v; write_rtlil vector.rtlil"

ghdl -a vectop.vhd
yosys -m ghdl -p "ghdl myvectop; write_rtlil vectop.rtlil"

yosys -p "read_rtlil vector.rtlil vectop.rtlil; write_cxxrtl vectop_gen.cpp"

clang++ -g -O3 -std=c++14 -I `yosys-config --datdir`/include/backends/cxxrtl/runtime  maintop.cpp -o tb_vec
