#!/bin/bash

rm -f /tmp/apa.tar

(cd ~/website/Deploy/Rpi/CryptPi/desktop_fpga ; tar -cvf /tmp/apa.tar .)

tar xvf /tmp/apa.tar

