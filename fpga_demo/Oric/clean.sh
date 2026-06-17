#!/bin/bash

mkdir -p old 

mv "#*" svcd vcdextract *.vcd *~ old/


pushd Oric_MiSTer/rtl

chmod u+w *

mkdir -p old 

chmod u+w old/*


mv sim_oricatmos *.cpp *.h *.vcd work-obj*.cf mk_oric.sh *.rtlil *.json *~ *.bin *.asc *.fs old/

popd


