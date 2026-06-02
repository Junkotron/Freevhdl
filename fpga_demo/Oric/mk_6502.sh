#!/bin/bash

# ice40 is implicit here...

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

if [ $MODE == "ghdl" ]; then
    exit 0
fi


do_yosys() {

    yosys -m ghdl -p "ghdl $FLAGS top_t65_system; write_rtlil my6502_ice40.rtlil"
    yosys < yo_my6502.txt

    VLOG_COMMON="modified_ice40.rtlil"

    yosys -p "read_rtlil $VLOG_COMMON; synth_ice40 -json my6502_ice40.json"

}

if [ $MODE != "pnr" ]; then
    do_yosys
fi


if [ $MODE == "synth" ]; then
    exit 0
fi


nextpnr-ice40 --hx8k --json my6502_ice40.json --pcf my6502.pcf --asc my6502.asc --package ct256 --pcf-allow-unconstrained

icepack my6502.asc my6502.bin



