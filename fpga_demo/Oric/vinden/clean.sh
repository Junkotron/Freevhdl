#!/bin/bash

mkdir -p old 

mv *.png *.jpg *.ppm "#*" svcd vcdextract *.vcd *~ old/

if [ -e Oric_MiSTer ]; then

    echo "cleaning the mister dir..."
    
    pushd Oric_MiSTer/rtl
    
    chmod u+w *
    
    mkdir -p old 
    
    chmod u+w old/*
    
    
    mv sim_oricatmos *.rom *.o *.cpp *.h *.vcd work-obj*.cf mk_oric.sh *.rtlil *.json *~ *.bin *.asc *.fs old/

    popd

fi


