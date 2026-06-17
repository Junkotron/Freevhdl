#!/bin/bash

if [ -e Oric_MiSTer ]; then
   echo "Oric already checked out.."
else
    echo "Checking out Oric.."
    git clone https://github.com/nikiiv/Oric_MiSTer.git
fi

VHDL_FILE_LIST="reset_logic.vhd microdisc_dummy.vhd apple2_disk/dpram_dummy.vhd apple2_disk/floppy_track_dummy.sv oricatmostop_ice40.vhd oricatmostop_gowin.vhd oricatmostop_sim.vhd divn.vhd"

MISC_FILE_LIST="oricatmos.pcf oricatmos.cst mk_oric.sh divn.vhd tristate.v yo_gowin.txt yo_ice40.txt sim_main_vhdl.cpp"

FILE_LIST="$MISC_FILE_LIST $VHDL_FILE_LIST"

pushd Oric_MiSTer/rtl
chmod u+w $FILE_LIST
popd


#(very) small patch to make work..
cat Oric_MiSTer/rtl/oricatmos.vhd | sed 's/inst_microdisc : work.Microdisc/inst_microdisc : entity work.Microdisc/g' > tmp_vhd.txt
cp tmp_vhd.txt Oric_MiSTer/rtl/oricatmos.vhd

for i in $FILE_LIST; do
    cp $i Oric_MiSTer/rtl/$i
done

rm -f tmp_vhd.txt

chmod a+rx Oric_MiSTer/rtl/mk_oric.sh

# Write protect in build dir
pushd Oric_MiSTer/rtl
chmod a-w $FILE_LIST
popd


pushd Oric_MiSTer/rtl

./mk_oric.sh sim all

popd

gcc -o svcd svcd.c


exit 0

cd Oric_MiSTer/rtl

./mk_oric.sh ice40 all

echo "Enter (for now) to build gowin"
read

./mk_oric.sh gowin all


