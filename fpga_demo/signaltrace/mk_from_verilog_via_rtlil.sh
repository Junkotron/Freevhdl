#!/bin/bash


SRC=st
TOP=sttop

yosys -p "read_verilog -sv divn.v st.v; write_rtlil st_ice40.rtlil"


yosys < yosys_in_rtlil.txt

yosys -p "read_rtlil result.rtlil; synth_ice40 -top $TOP -json from_rtlil_$SRC.json"

nextpnr-ice40 --hx8k --json from_rtlil_$SRC.json --pcf st_rtlil.pcf --asc st.asc --package ct256 --pcf-allow-unconstrained

icepack st.asc st.bin
