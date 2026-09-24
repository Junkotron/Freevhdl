#include "dasm.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// 💡 Vi låter konstruktorn starta helt tom och ren!
Disassembler::Disassembler() { //
} //

// 💡 DEN NYA GENERELLA RADEN: Låter main.cpp lägga till valfria fällor live!
void Disassembler::set_breakpoint(uint16_t pc) {
    breakpoints.insert(pc);
}

bool Disassembler::is_breakpoint(uint16_t pc) const { //
    return breakpoints.count(pc) > 0; //
} //



// I dasm.cpp
void Disassembler::print_cyc_step(unsigned long long step, uint16_t pc, OpcodeProfile prof, 
                                   uint8_t arg1, uint8_t arg2, 
                                   uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p) {
    // Din befintliga printf-logik för instruktionen här...
    // Lägg till eller behåll utskriften av registren, t.ex:
    // printf(" A:%02X X:%02X Y:%02X SP:%02X P:%02X\n", a, x, y, sp, p);
    std::stringstream asm_ss;
    asm_ss << prof.mnemonic;

    if (prof.bytes_count == 2) {
        asm_ss << std::hex << std::setw(2) << std::setfill('0') << (int)arg1;
    } 
    else if (prof.bytes_count == 3) {
        asm_ss << std::hex << std::setw(2) << std::setfill('0') << (int)arg2
               << std::hex << std::setw(2) << std::setfill('0') << (int)arg1;
    }

    std::cout << "\n        🔥 [CYC-HYBRID DISASM] #" << std::dec << step 
              << " vid PC: $" << std::hex << std::setw(4) << std::setfill('0') << pc << "\n"
              << "        👉 [ASM] " << std::left << std::setw(18) << std::setfill(' ') << asm_ss.str() << "\n\n";
}

int Disassembler::disassemble(uint8_t op, uint8_t arg1, uint8_t arg2, std::string& out_str) {
    std::stringstream ss;
    OpcodeProfile p = get_profile(op);
    ss << p.mnemonic;
    if (p.bytes_count == 2) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)arg1;
    }
    else if (p.bytes_count == 3) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)arg2
           << std::setw(2) << std::setfill('0') << (int)arg1;
    }
    out_str = ss.str();
    return p.bytes_count;
}

void Disassembler::print_step(unsigned long long step, uint16_t pc, uint8_t op, uint8_t arg1, uint8_t arg2, 
                       uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p, uint8_t bus_data, bool we) {
    std::string inst_str;
    disassemble(op, arg1, arg2, inst_str);
    std::cout << "Got: " << inst_str << std::endl;
    (void)step; (void)pc; (void)a; (void)x; (void)y; (void)sp; (void)p; (void)bus_data; (void)we;
}

int Disassembler::get_bytes_count(uint8_t op) {
    return get_profile(op).bytes_count;
}
