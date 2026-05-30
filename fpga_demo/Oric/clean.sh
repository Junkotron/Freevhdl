#!/bin/bash

mkdir -p old 

mv *~ old/


pushd Oric_MiSTer/rtl

mkdir -p old 

mv mk_oric.sh *.rtlil *.json *~ *.bin *.asc pack.fs old/

popd


