#!/bin/bash

if [ -e Oric_MiSTer ]; then
   echo "Oric already checked out.."
else
    echo "Checking out Oric.."
    git clone https://github.com/nikiiv/Oric_MiSTer.git
fi

VHDL_FILE_LIST="reset_logic.vhd divn.vhd my6502.vhd"

MISC_FILE_LIST="my6502.pcf mk_6502.sh divn.vhd yo_my6502.txt"

FILE_LIST="$MISC_FILE_LIST $VHDL_FILE_LIST"


pushd Oric_MiSTer/rtl
chmod u+w $FILE_LIST
popd

for i in $FILE_LIST; do
    cp $i Oric_MiSTer/rtl/$i
done


chmod a+rx Oric_MiSTer/rtl/mk_6502.sh

# Write protect in build dir
pushd Oric_MiSTer/rtl
chmod a-w $FILE_LIST
popd


cd Oric_MiSTer/rtl

./mk_6502.sh my6502 all

