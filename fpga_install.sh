#!/bin/bash

sudo apt update

sudo apt -y install curl make g++ pkg-config zlib1g-dev emacs libreadline-dev


#Start med någorlunda standard Ubuntu 25.10

sudo apt install git
sudo apt install gnat
git clone https://github.com/ghdl/ghdl
cd ghdl/
./configure --enable-synth
time make 
sudo make install
cd

git clone --recurse-submodules https://github.com/YosysHQ/yosys.git
cd yosys/
sudo apt-get -y install gawk git make python3 \
     lld bison clang libclang-cpp-dev flex libffi-dev \
     libfl-dev libreadline-dev pkg-config \
     tcl-dev zlib1g-dev graphviz xdot
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env
make config-clang
make config-gcc
time make
sudo make install
cd

git clone https://github.com/ghdl/ghdl-yosys-plugin.git
cd ghdl-yosys-plugin/
time make
sudo mkdir -p /usr/local/share/yosys/plugins
sudo cp ghdl.so /usr/local/share/yosys/plugins/
cd

