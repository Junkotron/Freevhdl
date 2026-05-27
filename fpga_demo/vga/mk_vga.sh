#!/bin/bash


do_yosys() {

    PLATTFORM=$1
    
    yosys -p "read_verilog example_0.v; write_rtlil vga_$PLATTFORM.rtlil"

    yosys -p "read_rtlil vga_$PLATTFORM.rtlil; synth_$PLATTFORM -json vga_$PLATTFORM.json"
    
    }

echo "now enter to synth ice40"
read

do_yosys ice40

echo "Enter to pnr ice40" 
read

nextpnr-ice40 --hx8k --json vga_ice40.json --pcf vga.pcf --asc vga.asc --package ct256 --pcf-allow-unconstrained


icepack vga.asc vga.bin



echo "now enter to synth gowin"
read

do_yosys gowin


echo "*** PNR ***"
DEVICE="GW2A-LV18PG256C7/I6"

BOARD=vga

PNRJSON=pnr_vga.json

echo "now enter to pnr gowin"
read

nextpnr-himbaechel --json vga_gowin.json \
                   --write $PNRJSON \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=GW2A-18


gowin_pack -d $DEVICE -o pack.fs $PNRJSON 
