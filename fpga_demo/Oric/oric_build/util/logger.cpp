#include "logger.hpp"
#include "argsparser.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>

namespace KMVLogger {

    void print_kislis(unsigned long long step, bool phi2, bool we, bool sync, 
                      uint64_t raw_regs, uint8_t bus_data, const std::string& asm_line) {
        uint16_t pc     = (raw_regs >> 48) & 0xFFFF;
        uint8_t  reg_sp = (raw_regs >> 32) & 0xFF;
        uint8_t  p      = (raw_regs >> 24) & 0xFF;
        uint8_t  reg_y  = (raw_regs >> 16) & 0xFF;
        uint8_t  reg_x  = (raw_regs >> 8)  & 0xFF;
        uint8_t  reg_a  = raw_regs & 0xFF;

        print_step(step, phi2, we, sync, pc, reg_a, reg_x, reg_y, reg_sp, p, bus_data, asm_line);
    }

    void print_step(unsigned long long step, bool phi2, bool we, bool sync,
                    uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p, 
                    uint8_t bus_data, const std::string& asm_line) {
        
        // Glesa ut loggen baserat på den globala argumentflaggans värde
        if (!KMVArgs::verbose && KMVArgs::glesning > 1 && (step % KMVArgs::glesning != 0)) {
            return;
        }
	
        char f_N = (p & 0x80) ? 'N' : '.';
        char f_V = (p & 0x40) ? 'V' : '.';
        char f_B = (p & 0x10) ? 'B' : '.';
        char f_D = (p & 0x08) ? 'D' : '.';
        char f_I = (p & 0x04) ? 'I' : '.';
        char f_Z = (p & 0x02) ? 'Z' : '.';
        char f_C = (p & 0x01) ? 'C' : '.';

        std::cout << "M-Cyk: " << std::setw(6) << std::setfill('0') << std::dec << step
                  << " | PHI2: " << phi2 << " WE: " << we << " SYNC: " << sync
                  << " | PC: $" << std::hex << std::setw(4) << std::setfill('0') << pc
                  << " | A: $" << std::setw(2) << std::setfill('0') << (int)a
                  << " X: $" << std::setw(2) << std::setfill('0') << (int)x
                  << " Y: " << std::setw(2) << std::setfill('0') << (int)y
                  << " SP: $" << std::setw(4) << std::setfill('0') << (0x0100 | sp)
                  << " | Bus: $" << std::setw(2) << std::setfill('0') << (int)bus_data
                  << " | Flags: [" << f_N << f_V << "." << f_B << f_D << f_I << f_Z << f_C << "]";
        
        if (!asm_line.empty()) {
            std::cout << " -> " << asm_line;
        }
        std::cout << std::endl;
    }

    void dump_oric_frame(const std::string& filename, unsigned long long step, const uint8_t* mem_ptr) {
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "❌ [ERROR] KMV Logger kunde inte skapa skärmdump: " << filename << std::endl;
            return;
        }

        out << "🖼️ [KMV CENTRAL LOGGER FRAMEGRABBER] Dump vid instruktion #" << std::dec << step << "\n";
        out << "+----------------------------------------+\n";

        const uint16_t screen_base = 0xBB00;

        for (int row = 0; row < 28; ++row) {
            out << "|";
            for (int col = 0; col < 40; ++col) {
                uint16_t addr = screen_base + (row * 40) + col;
                uint8_t raw_char = mem_ptr[addr];
                
                char c = raw_char & 0x7F;
                if (c < 32 || c > 126) {
                    c = ' '; 
                }
                out << c;
            }
            out << "|\n";
        }

        out << "+----------------------------------------+\n";
        out.close();
        std::cout << "📸 [FRAME GRABBED BY LOGGER] Skärmen dumpad till: " << filename << std::endl;
    }
}
