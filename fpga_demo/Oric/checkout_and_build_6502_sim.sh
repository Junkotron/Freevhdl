#!/bin/bash

if [ -e Oric_MiSTer ]; then
   echo "Oric already checked out.."
else
    echo "Checking out Oric.."
    git clone https://github.com/nikiiv/Oric_MiSTer.git
fi

VHDL_FILE_LIST="reset_logic.vhd divn.vhd my6502.vhd my6502_topsim.vhd"

MISC_FILE_LIST="mk_6502_sim.sh"

FILE_LIST="$MISC_FILE_LIST $VHDL_FILE_LIST"


pushd Oric_MiSTer/rtl
chmod u+w $FILE_LIST
popd

for i in $FILE_LIST; do
    cp $i Oric_MiSTer/rtl/$i
done


chmod a+rx Oric_MiSTer/rtl/mk_6502_sim.sh

# Write protect in build dir
pushd Oric_MiSTer/rtl
chmod a-w $FILE_LIST
popd


pushd Oric_MiSTer/rtl

./mk_6502_sim.sh

popd

# build simple tools

gcc -o vcdextract vcdextract.c
gcc -o svcd svcd.c

# extract signals for demo...

./vcdextract desired.txt 5ns < Oric_MiSTer/rtl/waveform6502.vcd > wavefilter6502.vcd

echo "If all went well you can now view some signals with ./svcd wavefilter6502.vcd"
echo "Other possible tools are gtkwave or VaporView"

