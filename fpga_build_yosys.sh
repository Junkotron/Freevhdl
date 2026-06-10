#!/bin/bash


#Start med någorlunda standard Ubuntu 25.10

git clone https://github.com/ghdl/ghdl
cd ghdl/
./configure --enable-synth
time make 
cd

git clone --recurse-submodules https://github.com/YosysHQ/yosys.git
cd yosys/
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env
cmake -B build
cd build
#make config-clang
#make config-gcc
time make
cd



