#!/bin/bash



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

ORIC="m6522.vhd ula.vhd video.vhd pravetz8d_fdc.vhd microdisc_dummy.vhd oricatmos.vhd oricatmostop.vhd"


for i in $ORIC; do
    ghdl -a -fexplicit -fsynopsys $FLAGS $i
done

echo "now enter to synth ice40"
read

yosys -m ghdl -p "ghdl $FLAGS oricatmostop; synth_ice40 -json oricatmos_ice40.json"

echo "Enter to pnr ice40" 
read

nextpnr-ice40 --hx8k --json oricatmos_ice40.json --pcf oricatmos.pcf --asc oricatmos.asc --package ct256 --pcf-allow-unconstrained


icepack oricatmos.asc oricatmos.bin



echo "now enter to synth gowin"
read

yosys -m ghdl -p "ghdl $FLAGS oricatmos; synth_gowin -json oricatmos_gowin.json"


echo "*** PNR ***"
DEVICE="GW2A-LV18PG256C7/I6"

BOARD=oricatmos

PNRJSON=pnr_oricatmos.json

echo "now enter to pnr gowin"
read

nextpnr-himbaechel --json oricatmos_gowin.json \
                   --write $PNRJSON \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=GW2A-18


gowin_pack -d $DEVICE -o pack.fs $PNRJSON 
