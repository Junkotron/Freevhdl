#!/bin/bash



# Used for all platforms
ghdl -a -fexplicit -fsynopsys ula.vhd 
ghdl -a -fexplicit -fsynopsys ulatop.vhd 



if false; then

echo "now enter to synth ice40"
read

yosys -m ghdl -p 'ghdl -fexplicit -fsynopsys ula; synth_ice40 -json ula_ice40.json'

echo "Enter to pnr" 
read

nextpnr-ice40 --hx8k --json ula_ice40.json --pcf ula.pcf --asc ula.asc --package ct256 --pcf-allow-unconstrained

icepack ula.asc ula.bin

fi


echo "now enter to synth gowin"
read

yosys -m ghdl -p "ghdl -fexplicit -fsynopsys ulatop; synth_gowin -json ula_gowin.json"


echo "*** PNR ***"
DEVICE="GW2A-LV18PG256C7/I6"

BOARD=ula

PNRJSON=pnr_ula.json

echo "now enter to pnr gowin"
read

nextpnr-himbaechel --json ula_gowin.json \
                   --write $PNRJSON \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=GW2A-18


gowin_pack -d $DEVICE -o pack.fs $PNRJSON 
