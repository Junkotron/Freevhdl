#!/bin/bash

PLATTFORM=$1

MODE=$2


# Used for all platforms
FLAGS="-fexplicit -fsynopsys --std=08"

rm -f work-obj93.cf

T65="T65_Pack.vhd T65_ALU.vhd T65_MCode.vhd T65.vhd"

for i in $T65; do
    ghdl -a $FLAGS T65/$i
done


COMPUTER="reset_logic.vhd divn.vhd my6502.vhd"


for i in $COMPUTER; do
    ghdl -a -fexplicit -fsynopsys $FLAGS $i
done

if  [ $PLATTFORM == "gowin" ]; then
    ghdl -a $FLAGS my6502_gowintop.vhd
fi



if [ $MODE == "ghdl" ]; then
    exit 0
fi


do_yosys() {

    # Avoid an extra russian doll for ice40 right now...
    if  [ $PLATTFORM == "gowin" ]; then
	TOPMODULE=top_t65_system_gowin
    else
	TOPMODULE=top_t65_system
    fi
    yosys -m ghdl -p "ghdl $FLAGS $TOPMODULE; write_rtlil my6502_$PLATTFORM.rtlil"
    
    
    yosys < yo_my6502_$PLATTFORM.txt
    
    VLOG_COMMON="modified_$PLATTFORM.rtlil"

    yosys -p "read_rtlil $VLOG_COMMON; synth_$PLATTFORM -json my6502_$PLATTFORM.json"

}

if [ $MODE != "pnr" ]; then
    do_yosys $PLATTFORM
fi


if [ $MODE == "synth" ]; then
    exit 0
fi


if  [ $PLATTFORM == "ice40" ]; then

    nextpnr-ice40 --hx8k --json my6502_ice40.json --pcf my6502.pcf --asc my6502.asc --package ct256 --pcf-allow-unconstrained

    icepack my6502.asc my6502.bin
fi

if  [ $PLATTFORM == "gowin" ]; then


    # tang "littlebee" 9k...
#    DEVICE="GW1NR-LV9QN88PC6/I5"
#    BOARD=littlebee9k
#    FAMILY=GW1N-9

    # tang 20k
    DEVICE="GW2AR-LV18QN88C8/I7"
    BOARD=tang20k
    FAMILY=GW2A-18C

    PNRJSON=pnr_simple_6502.json
    
    nextpnr-himbaechel --json my6502_gowin.json \
                       --write $PNRJSON \
                       --device $DEVICE \
                       --vopt cst=$BOARD.cst \
		       --vopt family=$FAMILY

    gowin_pack -d $DEVICE -o $BOARD"_BE_CAREFULL_AND_CHECK_THINGS".fs $PNRJSON 
fi





