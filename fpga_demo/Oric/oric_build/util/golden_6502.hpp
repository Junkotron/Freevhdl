#ifndef GOLDEN_6502_HPP
#define GOLDEN_6502_HPP


#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>

// 💡 DE SAKNADE BITARNA: Vi lägger till 8-bytesstrukturen här överst!
struct __attribute__((packed)) HugoTraceEntry {
    uint16_t pc;       // 2 bytes
    uint8_t  a;        // 1 byte
    uint8_t  x;        // 1 byte
    uint8_t  y;        // 1 byte
    uint8_t  sp;       // 1 byte
    uint8_t  p;        // 1 byte
    uint8_t  bus_data; // 1 byte
};


class Golden6502 {
public:
    // 💡 Vi döper om den råa vektorn till raw_mem
    std::vector<uint8_t> raw_mem; 
    uint16_t pc;
    uint8_t  a, x, y, sp, p;
    unsigned long long step_counter;
    std::string last_asm; // 💡 NYHET: Sparar senaste opkod-strängen för loggaren

    std::ifstream hugo_replay;
    bool          replay_mode = false;
  
    // 💡 Denna proxy-struktur gör att gamla "mem[addr]" i step.cpp 
    // automatiskt triggar vår I/O-tull vid läsning!
    struct MemoryProxy {
        Golden6502* cpu;
        uint16_t addr;

        // När koden i step.cpp gör: "uint8_t val = mem[addr];"
        operator uint8_t() const {
            if (addr >= 0x0300 && addr <= 0x03FF) {
                std::cout << "\n🚨 [IO-READ DETECTED] M-Cyk: " << std::dec << cpu->step_counter 
                          << " | Läser I/O-adress: $" << std::hex << addr 
                          << " | PC: $" << cpu->pc << std::endl;
            }
            return cpu->raw_mem[addr];
        }

        // När koden i step.cpp gör: "mem[addr] = val;"
        MemoryProxy& operator=(uint8_t val) {
            if (addr >= 0x0300 && addr <= 0x03FF) {
                std::cout << "\n📝 [IO-WRITE DETECTED] M-Cyk: " << std::dec << cpu->step_counter 
                          << " | Skriver $" << std::hex << (int)val 
                          << " till I/O-adress: $" << addr << " | PC: $" << cpu->pc << std::endl;
            }
            cpu->raw_mem[addr] = val;
            return *this;
        }
    };

    // Denna operator gör att "mem[addr]" returnerar vår smarta proxy istället för en rå byte
    MemoryProxy operator[](uint16_t addr) {
        return MemoryProxy{ this, addr };
    }

    Golden6502();
    void load_rom(const std::string& filename, uint16_t offset);
    void reset();
    bool step();

private:
    void update_nz(uint8_t val);
    void push_stack(uint8_t val);
    uint8_t pop_stack();
    void print_log_line(uint16_t inst_pc, const std::string& name, uint8_t bytes, uint16_t addr, bool is_imm);
};

#endif
