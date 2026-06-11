#!/bin/bash

do_all() {

    echo DEV=$DEVICE
    

    # Compile the verilog module into rtlil
    yosys -p " \
    	  read_verilog blinken.v; \
	  chparam -set N_LEDS $N_LEDS led; \
	  chparam -set LED_ON_WHEN $LED_ON_WHEN led; \
	  chparam -set DIVN $DIVN led; \
	  write_rtlil blinken.rtlil \
	  "

    # possible yo scripting here?
    cp blinken.rtlil blinken_patched.rtlil

    yosys -p "read_rtlil blinken_patched.rtlil; synth_gowin -json blinken.json"

    echo "*** PNR ***"


    # tang "littlebee" 9k...

    nextpnr-himbaechel --json blinken.json \
                       --write blinken_pnr.json \
                       --device $DEVICE \
                       --vopt cst=$BOARD.cst \
		       --vopt family=$FAMILY		   

    gowin_pack -d $FAMILY -o blinken_$BOARD.fs blinken_pnr.json
}

oflinfo() {

    echo "You should now be able to at your own peril download this"
    echo "to the $BOARD board with:"

    echo
    echo "sudo openFPGALoader -b $OFL blinken_$BOARD.fs"

}

DEVICE="GW1NR-LV9QN88PC6/I5"
BOARD=littlebee9k
FAMILY=GW1N-9
OFL=tangnano9k
N_LEDS=8
LED_ON_WHEN=1
DIVN=100_000_000

do_all

# tang 20k
DEVICE="GW2AR-LV18QN88C8/I7"
BOARD=tang20k
FAMILY=GW2A-18C
OFL=tangnano20k
N_LEDS=6
LED_ON_WHEN=0
DIVN=27_000_000


do_all

echo 

BOARD=littlebee9k
OFL=tangnano9k

oflinfo

echo 
echo "  ..  or  ... "
echo 

BOARD=tang20k
OFL=tangnano20k

oflinfo

