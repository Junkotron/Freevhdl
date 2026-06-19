#!/bin/bash

if [ $# -eq 0 ]; then
    echo "ok builds all for all ..."
    PLATF="all"
    JOB="all"
else if [ "$2" == "" ]; then
	 echo "Usage:"
	 echo "./$0"
	 echo "  or"
	 echo "./$0 [target] [job]"
	 exit 0
     else 
	 PLATF=$1
	 JOB=$2
     fi
fi

echo "Doing job: $JOB for $PLATF"


if [ -e Oric_MiSTer ]; then
   echo "Oric already checked out.."
else
    echo "Checking out Oric.."
    git clone https://github.com/nikiiv/Oric_MiSTer.git
fi

VHDL_FILE_LIST="reset_logic.vhd microdisc_dummy.vhd apple2_disk/dpram_dummy.vhd apple2_disk/floppy_track_dummy.sv oricatmostop_ice40.vhd oricatmostop_gowin.vhd oricatmostop_sim.vhd divn.vhd oricram.vhd"

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

gcc -o svcd svcd.c

pushd Oric_MiSTer/rtl

if [ "$PLATF" == "all" ]; then
./mk_oric.sh sim all
./mk_oric.sh ice40 all
./mk_oric.sh gowin all
else if [ $JOB == "all" ]; then
	 ./mk_oric.sh $PLATF all
     else
	 ./mk_oric.sh $PLATF $JOB
     fi
fi



popd
