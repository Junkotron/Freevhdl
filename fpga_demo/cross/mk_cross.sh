#!/bin/bash

SRC=vectop
TOP=myvectop


# Compile the verilog module into rtlil
yosys -p "read_verilog vector.v; write_rtlil cross_verilog.rtlil"

# Prepare the "header" for the vectop.vhd file in work-obj93.cf
ghdl -a -fexplicit -fsynopsys $SRC.vhd

# Do the same with the overlaying vhdl module using component inferal
yosys -m ghdl -p "ghdl -fexplicit -fsynopsys $TOP; write_rtlil cross_vhdl.rtlil"

# Take the combined files and do the acual synth:ing
yosys -p "read_rtlil cross_vhdl.rtlil cross_verilog.rtlil; synth_gowin -json from_vhdl_$SRC.json"

echo "*** PNR ***"
DEVICE="GW2A-LV18PG256C7/I6"

BOARD=vhdl_$SRC

PNRJSON=pnr_$SRC.json

nextpnr-himbaechel --json from_vhdl_$SRC.json \
                   --write $PNRJSON \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=GW2A-18


gowin_pack -d $DEVICE -o pack.fs $PNRJSON 
exit 0

