#!/bin/bash

git clone https://github.com/nikiiv/Oric_MiSTer.git

FILE_LIST="apple2_disk/dpram_dummy.vhd oricatmostop.vhd oricatmos.pcf oricatmos.cst mk_oric.sh"

#(very) small patch to make work..
cat Oric_MiSTer/rtl/oricatmos.vhd | sed 's/inst_microdisc : work.Microdisc/inst_microdisc : entity work.Microdisc/g' > tmp_vhd.txt

cp tmp_vhd.txt Oric_MiSTer/rtl/oricatmos.vhd

for i in $FILE_LIST; do
    cp $i Oric_MiSTer/rtl/$i
done

rm -f tmp_vhd.txt

chmod a+rx Oric_MiSTer/rtl/mk_oric.sh

cd Oric_MiSTer/rtl

./mk_oric.sh


