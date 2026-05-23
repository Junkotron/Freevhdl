#!/bin/bash

SRC=vector
TOP=myvector

ghdl -a -fexplicit -fsynopsys $SRC.vhd

yosys -m ghdl -p "ghdl -fexplicit -fsynopsys $TOP; synth_gowin -json from_vhdl_$SRC.json"

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

