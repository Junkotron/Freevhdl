#ifndef DASM_HPP
#define DASM_HPP

#pragma once
#include <cstdint>  
#include <string>
#include <unordered_set>

struct Disassembler {
    struct OpcodeProfile {
        const char* mnemonic;
        int total_cycles;
        int arg1_cycle;  
        int arg2_cycle;  
        int bytes_count; 
    };

private:
    std::unordered_set<uint16_t> breakpoints;

public:
  Disassembler();

  void set_breakpoint(uint16_t pc);


  bool is_breakpoint(uint16_t pc) const;

// I dasm.hpp
    static void print_cyc_step(unsigned long long step, uint16_t pc, OpcodeProfile prof, 
			     uint8_t arg1, uint8_t arg2, 
			     uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p);
    static OpcodeProfile get_profile(uint8_t op);
    
                               
    static int disassemble(uint8_t op, uint8_t arg1, uint8_t arg2, std::string& out_str);
    static void print_step(unsigned long long step, uint16_t pc, uint8_t op, uint8_t arg1, uint8_t arg2, 
                           uint8_t a, uint8_t x, uint8_t y, uint8_t sp, uint8_t p, uint8_t bus_data, bool we);
    static int get_bytes_count(uint8_t op);
};

#endif
