#!/bin/bash

mkdir -p old 

mv *~ old/


pushd Oric_MiSTer/rtl

chmod u+w *

mkdir -p old 

chmod u+w old/*


mv mk_oric.sh *.rtlil *.json *~ *.bin *.asc *.fs old/

popd


