#include "golden_6502.hpp"
#include "logger.hpp"
#include "argsparser.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // Svart bälte i gemensamma flaggor
    KMVArgs::parse(argc, argv, {}); 

    Golden6502 emu;
    emu.load_rom("BASIC11A.rom", 0xC000); 
    emu.reset();

    // HUGO REPLAY: Kontrollera om flaggan -r / --replay är aktiv
    if (KMVArgs::replay) {
        emu.hugo_replay.open("kislis.trace", std::ios::binary);
        if (!emu.hugo_replay.is_open()) {
            emu.hugo_replay.open("../kislis.trace", std::ios::binary);
        }

        if (emu.hugo_replay.is_open()) {
            emu.replay_mode = true;
            std::cout << "🏁 [HUGO TRACE-DRIVEN REPLAY ACTIVE] Läser kislis.trace i superfart...\n" << std::endl;
            
            // 💡 FIXEN: Sug in hårdvarans exakta start-DNA (flaggor & register!)
            HugoTraceEntry start_entry;
            if (emu.hugo_replay.read(reinterpret_cast<char*>(&start_entry), sizeof(HugoTraceEntry))) {
                std::cout << "🎯 [START-SYNC] Adopterar kislis fulla CPU-tillstånd för Steg 1:\n"
                          << "   -> PC: $" << std::hex << start_entry.pc 
                          << " | A: $" << (int)start_entry.a 
                          << " | X: $" << (int)start_entry.x 
                          << " | P: %" << (int)start_entry.p << std::endl;
                
                // Teleportera hela mjukis CPU-tillstånd till hårdvarans sanna universum
                emu.pc = start_entry.pc;
                emu.a  = start_entry.a;
                emu.x  = start_entry.x;
                emu.y  = start_entry.y;
                emu.sp = start_entry.sp;
                emu.p  = start_entry.p; // 💥 Flaggorna laddas med kiselsanning!
                
                emu.hugo_replay.seekg(0, std::ios::beg); // Backa filen till start
            }
        } else {
            std::cerr << "❌ [HUGO CRITICAL ERROR] Hittade inte 'kislis.trace' på disken!" << std::endl;
            return 1;
        }
    }
    
    // =====================================================================
    // 💡 NY INTELLIGENT LOCKSTEP-LOOPER (Verifierar mjukis mot kislis!)
    // =====================================================================
    if (emu.replay_mode) {
        // Helt global och ren – precis som i din hpp-fil!
        HugoTraceEntry kisel_entry;
        unsigned long long replay_steps = 0;

        std::cout << "🏁 [HUGO LOCKSTEP VERIFIER ACTIVE] Kör mjukis parallellt mot kislis.trace...\n" << std::endl;

        // Läs nästa facit-rad från kislis binärspår
        while (emu.hugo_replay.read(reinterpret_cast<char*>(&kisel_entry), sizeof(HugoTraceEntry))) {
            replay_steps++;

            // Innan mjukis kör steget, tvingar vi minnet på just den adressen att matcha facit
            emu.raw_mem[emu.pc] = kisel_entry.bus_data;

            // 💡 SYNCHRONISERING: Spara mjukis sanna adress PRECIS INNAN instruktionen exekveras!
            uint16_t mjukis_pc_innan = emu.pc;

            // 💥 KÖR MJUKIS EGEN UTMANARE: Detta väcker liv i din MemoryProxy och dess IO-loggar!

	    if (!emu.step()) {
                std::cout << "⚠️ Replay stoppades av mjukis-avkodaren (BRK eller okänd opkod)." << std::endl;
                break;
            }
	    
            // 🔍 UTÖKAD SKOTTSÄKER LOCKSTEP-KONTROLL: Matchar mjukis HELA CPU-tillståndet?
            bool divergens = false;
            uint8_t mjukis_opcode = emu.raw_mem[mjukis_pc_innan];
	    
            if (mjukis_pc_innan != kisel_entry.pc) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP PC-DIVERGENS DETEKTERAD VID INSTRUCTION #" << std::dec << replay_steps << "! 🔴 🚨🚨\n"
                          << "👉 Mjukis stod på: $" << std::hex << mjukis_pc_innan << "\n"
                          << "👉 Kislis spår säger: $" << std::hex << kisel_entry.pc << "\n";
                divergens = true;
            }

            // 💡 SLUT PÅ NAIVITETEN: Jämför mjukis tänkta opkod mot kislis sanna bus-data!
            else if (mjukis_opcode != kisel_entry.bus_data) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP OPKOD-DIVERGENS DETEKTERAD VID INSTRUCTION #" << std::dec << replay_steps << "! 🔴 🚨🚨\n"
                          << "👉 PC: $" << std::hex << mjukis_pc_innan << "\n"
                          << "👉 Mjukis försöker köra opkod: $" << std::hex << (int)mjukis_opcode << "\n"
                          << "👉 Kislis buss visar faktiskt: $" << std::hex << (int)kisel_entry.bus_data << "\n"
                          << "💡 (Detta betyder att mjukis ROM och kislis ROM inte matchar på denna adress!)\n";
                divergens = true;
            }	    

            // 🔍 SKOTTSÄKER LOCKSTEP-KONTROLL: Nu jämför vi allt!
            
            if (mjukis_pc_innan != kisel_entry.pc) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP PC-DIVERGENS DETEKTERAD! 🔴 🚨🚨\n";
                divergens = true;
            }
            // 💡 BEVAKA STACKPEKAREN (Här ska det smälla direkt på steg 2!)
	    /*

	    else if (emu.sp != kisel_entry.sp) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP STACK-DIVERGENS DETEKTERAD VID INSTRUCTION #" << std::dec << replay_steps << "! 🔴 🚨🚨\n"
                          << "👉 Mjukis SP: $" << std::hex << (0x0100 | emu.sp) << "\n"
                          << "👉 Kislis SP: $" << std::hex << (0x0100 | kisel_entry.sp) << "\n";
                divergens = true;
            }
	    */
	    /*
            else if (emu.x != kisel_entry.x) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP REGISTER-X DIVERGENS DETEKTERAD! 🔴 🚨🚨\n";
                divergens = true;
            }
	    */
	    /*
            // Kontrollera Ackumulatorn (A)
            else if (emu.a != kisel_entry.a) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP REGISTER-A DIVERGENS DETEKTERAD VID INSTRUCTION #" << std::dec << replay_steps << "! 🔴 🚨🚨\n"
                          << "👉 Mjukis A: $" << std::hex << (int)emu.a << "\n"
                          << "👉 Kislis A: $" << std::hex << (int)kisel_entry.a << "\n";
                divergens = true;
            }


	    // Kontrollera Statusflaggorna (P) - bortse från oanvända Bit 5 (alltid 1) och Break-flaggan Bit 4
            else if ((emu.p & 0xCF) != (kisel_entry.p & 0xCF)) {
                std::cout << "\n🚨🚨 🔴 LOCKSTEP FLAGGDIVERGENS (P) DETEKTERAD VID INSTRUCTION #" << std::dec << replay_steps << "! 🔴 🚨🚨\n"
                          << "👉 Mjukis P: %" << std::hex << (int)emu.p << "\n"
                          << "👉 Kislis P: %" << std::hex << (int)kisel_entry.p << "\n";
                divergens = true;
            }

	    */
	    
            if (divergens) {
                std::cout << "💥 Bryter uppspelningen omedelbart för felsökning." << std::endl;
                break;
            }

            // 💡 EN ENDA STRÖMLINJEFORMAD RAD: Filmrulle + Fullständig CPU-status + Opkod!
            if (replay_steps % KMVArgs::glesning == 0) {
                // Skapa prefixet med filmrullar
                std::string prefix_with_asm = "🎞️  -> " + emu.last_asm;

                KMVLogger::print_step(
                    replay_steps, 
                    false, false, true, // PHI2, WE, SYNC
                    mjukis_pc_innan, 
                    emu.a, emu.x, emu.y, emu.sp, emu.p, 
                    kisel_entry.bus_data, 
                    prefix_with_asm // 💥 BOOM! Skickas med som sista textfältet
                );
            }

        }
	
        std::cout << "\n🏁 [HUGO SUCCESS] Lockstep-verifiering klar! Totalt: " 
                  << std::dec << replay_steps << " instruktioner kollade mot hårdvaran." << std::endl;
        
        KMVLogger::dump_oric_frame("oric_screen_replay.txt", replay_steps, emu.raw_mem.data());

    } else {
        // Standard Fristående Mörkerkörning (utan -r)
        std::cout << "🕵️‍♂️ [SPION MODE] Kör mjukis i fristående isolerat läge." << std::endl;
        while (emu.step()) {
            // Kör på...
        }
    }

    if (emu.replay_mode) {
        emu.hugo_replay.close();
    }
    return 0;
}
