#!/bin/bash


# Compile the verilog module into rtlil
yosys -p "read_verilog blinken.v; write_rtlil blinken.rtlil"

# possible yo scripting here?
cp blinken.rtlil blinken_patched.rtlil

yosys -p "read_rtlil blinken_patched.rtlil; synth_gowin -json blinken.json"

echo "*** PNR ***"


# tang "littlebee" 9k...
DEVICE="GW1NR-LV9QN88PC6/I5"
BOARD=littlebee9k
FAMILY=GW1N-9

# tang 20k
#DEVICE="GW2A-LV18PG256C7/I6"
#BOARD=tang20k
#FAMILY=GW2A-18

nextpnr-himbaechel --json blinken.json \
                   --write blinken_pnr.json \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=$FAMILY

		   

gowin_pack -d $FAMILY -o blinken_$BOARD.fs blinken_pnr.json

