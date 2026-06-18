#include <iostream>
#include "oricatmos_sim.h"

// 4 .vcd
#include <cxxrtl/cxxrtl_vcd.h>
#include <fstream>
  
int time_steps=0;

cxxrtl_design::p_oricatmostop__sim top_design;

void set_master_clock(bool val)
{
  // Drive top-level inputs (Verify underscore quantity in gen_design.h)
  top_design.p_CLK__24MHz.set<uint8_t>(val);

  // For the sceptic
  // std::cout << "Simulation Input:  " << (int)top_design.p_CLK__24MHz.get<uint8_t>() << std::endl;
}

void set_reset(bool val)
{
  top_design.p_RESET.set<uint8_t>(val);  
}

int main() {

  // Öppna en filström för att spara vcd-filen
  std::ofstream vcd_file("waveforms.vcd");

  // 24 MHz 
  vcd_file << "$timescale 21ns $end\n"; 
  
  // Skapa VCD-skrivaren och spara designens signalhierarki
  cxxrtl::vcd_writer vcd;
  //  top_design.debug_info(vcd.debug_item());

  // 1. Skapa en behållare för signalerna
  cxxrtl::debug_items items;

  cxxrtl::debug_scopes scopes;

  // 3. Anropa debug_info med pekare till objekten samt en tom rotsökväg ""
  top_design.debug_info(&items, &scopes, "");
  //top_design.debug_info(items); // when verilog only

  // 3. Registrera dessa signaler i din VCD-skrivare
  vcd.add(items);

  std::cout << "Sim done init..." << std::endl;

  set_reset(true);
  top_design.step();

  // todo have this in real time calculated instead 1000 = about 40 us
  for (int i=0;i<2000;i++)
    {
      if (i==20) set_reset(false);
      
      set_master_clock(true);
    
      // Evaluate the simulation. This calls your custom C++ blackbox eval() automatically!
      top_design.step();

      // Dumpa tillstånd efter en halv klockcykel
      vcd.sample(time_steps++);
      vcd_file << vcd.buffer; // 2. Skriv ut den till filen
      vcd.buffer.clear();     // 3. Töm inför nästa varv



      set_master_clock(false);
    
      // Evaluate the simulation. This calls your custom C++ blackbox eval() automatically!
      top_design.step();


      // Dumpa tillstånd efter en halv klockcykel
      vcd.sample(time_steps++);
      vcd_file << vcd.buffer; // 2. Skriv ut den till filen
      vcd.buffer.clear();     // 3. Töm inför nästa varv

    }

  vcd_file.flush(); // Sparar bufferten till filen

  return 0;
}
