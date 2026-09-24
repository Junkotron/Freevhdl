#pragma once
#include <string>
#include <cstdint>

namespace KMVLogger {
   // Central funktion för att skriva ut register (delas under huven)
    void print_step(unsigned long long step, bool phi2, bool we, bool sync,
                    uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p, 
                    uint8_t bus_data, const std::string& asm_line = "");

    // Generisk skärmdump
    void dump_oric_frame(const std::string& filename, unsigned long long step, const uint8_t* mem_ptr);

    // Kislis-ingång (sim_oricatmos)
    void print_kislis(unsigned long long step, bool phi2, bool we, bool sync, 
                      uint64_t raw_regs, uint8_t bus_data, const std::string& asm_line = "");
}
