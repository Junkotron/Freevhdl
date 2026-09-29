#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <fstream>

#include "util/golden_6502.hpp"
#include "util/argsparser.hpp"
#include "dasm.hpp"
#include "oric_rig.hpp"
#include "util/logger.hpp"
#include "util/argsparser.hpp"

void monitor_irq_edges(bool current_state, unsigned long long step_counter, uint16_t pc) {
  static bool last_state = false;
  static bool initialized = false;

  if (!initialized) {
    last_state = current_state;
    initialized = true;
    std::cout << "🔍 [IRQ MONITOR] Initialtillstånd vid start: "
	      << (current_state ? "HIGH (1)" : "LOW (0)")
	      << " | Steg: " << step_counter
	      << " | PC: $" << std::hex << pc << std::dec << "\n";
    return;
  }

  if (current_state != last_state) {
    std::cout << "⚡ [IRQ FLANK] Förändring: "
	      << (last_state ? "HIGH (1)" : "LOW (0)") << " ➔ "
	      << (current_state ? "HIGH (1)" : "LOW (0)")
	      << " | Steg: " << std::dec << step_counter
	      << " | PC: $" << std::hex << pc << std::dec << "\n";
    last_state = current_state;
  }
}




int main(int argc, char* argv[]) {

  // Svart bälte i gemensamma flaggor (-v och -n tas om hand här!)
  KMVArgs::parse(argc, argv, {"--trace-floppy", "--turbo"});

  if (KMVArgs::has_flag("--trace-floppy")) { /* ... */ }

  OricRig rig;
  Disassembler dasm;

// 💡 STENHÅRD STRATEGI: Ingen gissad default!
  if (KMVArgs::load_custom!="") { // [6, 7]
      // Om användaren skickade med -l, kör vi din anpassade testremsa [6]
      if (!rig.load_rom_image(KMVArgs::load_custom, 0xC000)) { // [6]
	std::cerr << "❌ [MAIN] Avbryter! Du bad om -l men " << KMVArgs::load_custom << "saknas på disken." << std::endl; // [6]
          return 1; 
      }
      // Aktivera ditt ROM-gränssnitt på kislet
      rig.set_rom_select(0b11); 
  } else {
      // Standardläge: Ingen förladdning sker, vi kör orörd hårdvarukonfiguration
      std::cout << "🕵️‍♂️ [MAIN] Ingen extern förladdning begärd. Sätter ROM-väljaren till \"00\"." << std::endl;
      rig.set_rom_select(0b00);
  }


  std::cout << "Sparkar igång SKOTTSÄKER CYKEL-AVBILDAD STATEMASKIN MED BREAKPOINTS..." << std::endl;
  rig.hardware_reset();

  // 💡 Vi sätter fällan på startadressen eller din valda testadress!
  dasm.set_breakpoint(0xF88F);

  // 💡 HUGO RECORD: Öppna den binära filströmmen för det steganografiska facitet!
  std::ofstream h_trace("util/kislis.trace", std::ios::binary);
  if (!h_trace.is_open()) {
      std::cerr << "❌ [HUGO ERROR] Kunde inte skapa spårfilen util/kislis.trace!" << std::endl;
      return 1;
  }

  unsigned long long total_steps = 0;
  unsigned long long step_counter = 0;
  bool     last_phi2     = false;

  Disassembler::OpcodeProfile active_prof = { "NOP", 2, -1, -1, 1 };
  uint16_t      inst_pc      = 0;
  bool          breakpoint_triggered = false;

  while (true) {

    rig.clock_master_high();
    bool phi2 = rig.get_cpu_clk();

    if (last_phi2 && !phi2) {
        if (rig.is_sync()) {
            step_counter++;

            uint8_t current_op = rig.get_bus_data();
            active_prof = Disassembler::get_profile(current_op);

            uint64_t current_regs = rig.get_cpu_regs();
            inst_pc = (current_regs >> 48) & 0xFFFF;

            // 💡 HUGO RECORD: Packa och dumpa kisel-sanningen till disken live vid SYNC! [18.1]
            HugoTraceEntry entry;
            entry.pc       = inst_pc;
            entry.a        = current_regs & 0xFF;
            entry.x        = (current_regs >> 8) & 0xFF;
            entry.y        = (current_regs >> 16) & 0xFF;
            entry.sp       = (current_regs >> 32) & 0xFF;
            entry.p        = (current_regs >> 24) & 0xFF; 
            entry.bus_data = current_op; 

            h_trace.write(reinterpret_cast<const char*>(&entry), sizeof(HugoTraceEntry));
            
            // 💡 GARANTERAD FLUSH: Töm bufferten till disken direkt så Ctrl+C aldrig sabbar filen!
            h_trace.flush();

            // 💡 DYNAMISK LOGGNING VIA KOMMANDORADEN (-v och -n)
            if (KMVArgs::verbose || breakpoint_triggered || (step_counter % KMVArgs::glesning == 0)) {
	      // 💡 Ändra rad 89 i din main.cpp till detta:
	      KMVLogger::print_kislis(step_counter, phi2, rig.get_we(), rig.is_sync(), current_regs, current_op, active_prof.mnemonic);

            }

            // 💡 BREAKPOINT-DETEKTION
            if (dasm.is_breakpoint(inst_pc)) {
                std::cout << "\n🛑🛑 🔴 BREAKPOINT TRÄFFAD VID PC: $" << std::hex << inst_pc << "! 🔴 🛑🛑\n";
                breakpoint_triggered = true;
            }

	    // 🎯 Kör övervakaren på varje SYNC (eller varje klockcykel om du föredrar det)
	    monitor_irq_edges(rig.get_via_irq(), step_counter, inst_pc);
            // Debugger-handbromsen vid träff
            if (breakpoint_triggered) {
                std::cout << "--- [KMV DEBUGGER] Tryck ENTER för nästa CPU-steg ---";
                std::cin.get();
                breakpoint_triggered = false; // Återställ så vi kan stega vidare
            }
        }
    }
    last_phi2 = phi2;
    rig.clock_master_low();
    total_steps++;

    if (total_steps % 500000 == 0) {
        std::cout << "⏳ [KMV HEARTBEAT] Loopen tuggar! Totala kiselsteg: " << std::dec << total_steps
                  << " | CPU Instruktioner: " << step_counter << std::endl;
    }
}

  h_trace.close();
  return 0;
}
