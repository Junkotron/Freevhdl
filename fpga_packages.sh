#!/bin/bash

sudo apt update

sudo apt -y install curl make g++ pkg-config zlib1g-dev emacs libreadline-dev

sudo apt -y install gnat
sudo apt-get -y install gawk git make python3 \
     lld bison clang libclang-cpp-dev flex libffi-dev \
     libfl-dev libreadline-dev pkg-config \
     tcl-dev zlib1g-dev graphviz xdot \
     gtkwave

sudo apt -y install \
     git \
     cmake \
     libboost-filesystem-dev \
     libboost-thread-dev \
     libboost-dev \
     libboost-program-options-dev \
     libboost-iostreams-dev \
     libboost-python-dev \
     libeigen3-dev \
     libftdi1-dev \
     python3-pip \
     python3-msgpack \
     pybind11-dev \
     python3-msgspec

