#!/bin/bash

curl https://github.com/trabucayre/openFPGALoader.git

cd openFPGALoader/

mkdir build
cd build

cmake ..

cmake --build .

