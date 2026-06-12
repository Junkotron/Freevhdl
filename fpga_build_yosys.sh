#!/bin/bash

# TODO: Automate for raspberry:
#
# We need this lvm thing since the defaults use native x86
# things
#
# sudo apt update
# sudo apt install llvm-dev clang libzstd-dev zlib1g-dev
# ./configure --enable-synth --prefix=/usr/local --with-llvm-config


#Start med någorlunda standard Ubuntu 26.04 LTS

git clone https://github.com/ghdl/ghdl
cd ghdl/
ARCH=`uname -m`
if [ $ARCH == "aarch64" ]; then
    ./configure --enable-synth --prefix=/usr/local --with-llvm-config
else
    ./configure --enable-synth
fi
time make -j$(nproc)
cd

git clone --recurse-submodules https://github.com/YosysHQ/yosys.git
cd yosys/
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env
cmake -B build
cd build
#make config-clang
#make config-gcc
time make -j$(nproc)
cd



