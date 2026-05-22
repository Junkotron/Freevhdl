#!/bin/bash


cd


mkdir Nand
cd Nand/

cp /home/Nand.v .
cp /home/Nand.vhd .


ghdl -a Nand.vhd 
yosys -m ghdl -p 'ghdl nand_gate; synth_ice40 -json Nand_from_vhdl.json'

yosys -q -p "synth_ice40 -top nand_gate -json Nand_from_verilog.json" Nand.v


nextpnr-ice40 --hx8k --json Nand_from_vhdl.json --pcf nandy.pcf --asc nandy_vhdl.asc

nextpnr-ice40 --hx8k --json Nand_from_verilog.json --pcf nandy_verilog.pcf --asc nandy_verilog.asc

icepack nandy_vhdl.asc nandy_vhdl.bin
icepack nandy_verilog.asc nandy_verilog.bin


