#!/bin/bash

PLATTFORM=$1

MODE=$2


# Used for all platforms
FLAGS="-fexplicit -fsynopsys --std=08"

rm -f work-obj93.cf

APPLE="dpram_dummy.vhd disk_ii_rom.vhd drive_ii.vhd disk_ii.vhd"

for i in $APPLE; do
    ghdl -a $FLAGS apple2_disk/$i
done

T65="T65_Pack.vhd T65_ALU.vhd T65_MCode.vhd T65.vhd"

for i in $T65; do
    ghdl -a $FLAGS T65/$i
done


DISKS="BASIC10.vhd BASIC11.vhd BASIC22.vhd DIAG10.vhd MICRODIS.vhd ORIC1SDCARD.vhd PRAVETZ8D.vhd PRAVETZ8D_FDC.vhd BASIC11A.vhdl BASIC11B.vhdl"

for i in $DISKS; do
    ghdl -a $FLAGS rom/$i
done

ORIC="divn.vhd m6522.vhd ula.vhd pravetz8d_fdc.vhd microdisc_dummy.vhd oricatmos.vhd oricatmostop_ice40.vhd oricatmostop_gowin.vhd"


for i in $ORIC; do
    ghdl -a -fexplicit -fsynopsys $FLAGS $i
done


do_yosys() {

    
    yosys -p "read_verilog keyboard.sv; write_rtlil keyboard_$PLATTFORM.rtlil"

    if [ $PLATTFORM == "ice40" ]; then
	yosys -p "read_verilog tristate.v; write_rtlil tristate_$PLATTFORM.rtlil"
    fi

    yosys -m ghdl -p "ghdl $FLAGS oricatmostop_$PLATTFORM; write_rtlil oricatmostop_$PLATTFORM.rtlil"

    VLOG_COMMON="oricatmostop_$PLATTFORM.rtlil keyboard_$PLATTFORM.rtlil"
    if [ $PLATTFORM == "ice40" ]; then
	VLOG_PLATTFORM="tristate_ice40.rtlil"
    fi
    if [ $PLATTFORM == "gowin" ]; then
	# TODO
	VLOG_PLATTFORM=""
    fi

    yosys -p "read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; synth_$PLATTFORM -json oricatmostop_$PLATTFORM.json"

}

if [ $MODE != "pnr" ]; then
    do_yosys $PLATTFORM
fi


if [ $MODE == "synth" ]; then
    exit 0
fi

if [ $PLATTFORM == "ice40" ]; then
    nextpnr-ice40 --hx8k --json oricatmostop_ice40.json --pcf oricatmos.pcf --asc oricatmos.asc --package ct256 --pcf-allow-unconstrained

    icepack oricatmos.asc oricatmos.bin
fi


if [ $PLATTFORM == "gowin" ]; then

    DEVICE="GW2A-LV18PG256C7/I6"
    
    BOARD=oricatmos
    
    PNRJSON=pnr_oricatmos.json
    
    nextpnr-himbaechel --json oricatmostop_gowin.json \
                       --write $PNRJSON \
                       --device $DEVICE \
                       --vopt cst=$BOARD.cst \
		       --vopt family=GW2A-18


    gowin_pack -d $DEVICE -o oricatmos.fs $PNRJSON 
fi
