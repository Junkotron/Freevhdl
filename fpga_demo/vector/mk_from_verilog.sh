#!/bin/bash


SRC=vectop
TOP=myvectop

yosys -D LEDS_NR=8 -D INV_BTN=0 -p "read_verilog vector.v $SRC.v; synth_gowin -top $TOP -json from_verilog_$SRC.json -family gw2a"


DEVICE="GW2A-LV18PG256C7/I6"

BOARD=verilog_$SRC
echo "*** PNR ***"
PNRJSON=pnr_$SRC.json

nextpnr-himbaechel --json from_verilog_$SRC.json \
                   --write $PNRJSON \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=GW2A-18


gowin_pack -d $DEVICE -o pack.fs $PNRJSON 
# gowin_unpack -d $DEVICE -o unpack.v pack.fs
# yosys -p "read_verilog -lib +/gowin/cells_sim.v; clean -purge; show" unpack.v
exit 0
openFPGALoader -b $BOARD pack.fs

