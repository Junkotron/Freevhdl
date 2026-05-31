#!/bin/bash


SRC=st
TOP=sttop

yosys < yosys_in.txt

yosys -p "read_verilog -sv result_noedit.v; synth_ice40 -top $TOP -json from_verilog_$SRC.json"



nextpnr-ice40 --hx8k --json from_verilog_$SRC.json --pcf st.pcf --asc st.asc --package ct256 --pcf-allow-unconstrained

icepack st.asc st.bin
