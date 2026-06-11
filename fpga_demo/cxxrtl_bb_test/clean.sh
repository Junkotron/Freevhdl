#!/bin/bash


mkdir -p old

mv gen_*.cpp gen_*.h *.o work*.cf *.rtlil *~ sim_executable sim_vhdl_executable old
