
# pipe this to yosys...

read_rtlil oricatmostop_ice40.rtlil
hierarchy -top oricatmostop_ice40
proc
ls

cd oricatmostop_ice40
select oric.ula_phi2

expose
write_rtlil modified_ice40.rtlil

