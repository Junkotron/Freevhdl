// I util/golden_6502.cpp
#include "golden_6502.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib> // För std::exit
#include "argsparser.hpp"
#include "logger.hpp"

Golden6502::Golden6502() : raw_mem(65536, 0x00), pc(0), a(0), x(0), y(0), sp(0), p(0x20), step_counter(0) {}


void Golden6502::load_rom(const std::string& filename, uint16_t offset) {
    std::ifstream file(filename, std::ios::binary);
    
    // 💡 SÄKRINGEN: Om filen inte kunde öppnas, bli arg och tvärstanna!
    if (!file.is_open()) {
        std::cerr << "\n❌ [CRITICAL ERROR] Kunde inte öppna ROM-filen: \"" << filename << "\"\n"
                  << "👉 Kontrollera att filen ligger på rätt ställe och att du kör från rätt katalog!" 
                  << std::endl;
        std::exit(1);
    }

    // Om filen fanns, läs in den precis som vanligt i raw_mem
    file.read(reinterpret_cast<char*>(&raw_mem[offset]), 16384);
    file.close();
    
    std::cout << "💾 [ROM LOADED] \"" << filename << "\" inläst till $" << std::hex << offset << std::endl;
}




void Golden6502::reset() {
    pc = raw_mem[0xFFFC] | (raw_mem[0xFFFD] << 8);
    step_counter = 0;
    p = 0x24; 
}

void Golden6502::update_nz(uint8_t val) {
    p &= ~0x82; 
    if (val == 0)   p |= 0x02;
    if (val & 0x80) p |= 0x80;
}

void Golden6502::push_stack(uint8_t val) {
    raw_mem[0x0100 | sp] = val;
    sp--;
}

uint8_t Golden6502::pop_stack() {
    sp++;
    return raw_mem[0x0100 | sp];
}



void Golden6502::print_log_line(uint16_t inst_pc, const std::string& name, uint8_t bytes, uint16_t addr, bool is_imm) {

  last_asm=name;
  
    // 💡 AUTOMATISK FRAMEGRABBER-TULL INUTI LOGGKODEN
    if (step_counter == 1) {
        // Skicka med filnamn, nuvarande steg, och en direktpekare till mjukis-RAM
        KMVLogger::dump_oric_frame("oric_screen.txt", step_counter, &raw_mem[0]);
    }

    // Din vanliga glesningsspärr och utskrift rullar på precis som vanligt här under
    if (step_counter % KMVArgs::glesning != 0) {
        return; 
    }
    
    // 💡 HÄR FLÄTAR VI IN FILMRULLEN UTAN ATT TA BORT NÅGOT:
    // Om vi kör i replay-läge (-r), lägger vi till en filmrulle framför opkodens namn!
    std::string final_name = name;
    if (replay_mode) {
        final_name = "🎞️  -> " + name;
    }
}
