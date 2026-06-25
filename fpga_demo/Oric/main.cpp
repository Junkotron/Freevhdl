// g++ -g -I `yosys-config  --datdir`/include/backends/cxxrtl/runtime cpu_only_sim.cc main.cpp snapshot_utils.cpp -o gipsvagga



#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <cxxrtl/cxxrtl.h>

#include "snapshot_utils.h"

// Det globala, simulerade minnesutrymmet för vår Oric (64 Kilobyte)
std::vector<uint8_t> oric_memory(65536, 0);

// Bussenjektor-blackboxen i C++
void drive_memory_bus(cxxrtl_design::p_t65__Brtl &cpu) {
    uint32_t raw_addr = cpu.p_a.get<uint32_t>();
    uint16_t addr = raw_addr & 0xFFFF;

    bool is_write = (cpu.p_r__w__n.get<bool>() == false);

    if (is_write) {
        uint8_t data_to_write = cpu.p_do.get<uint8_t>();
        oric_memory[addr] = data_to_write;
    } else {
        uint8_t data_to_read = oric_memory[addr];
        cpu.p_di.set<uint8_t>(data_to_read);
    }
}

bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

int main() {
    size_t kallstart_cycles = 100; 
    bool use_external_ram = true;  
    uint8_t dummy_memory = 0;

    cxxrtl_design::p_t65__Brtl top; 
    cxxrtl::debug_items all_items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&all_items, &scopes, "");  

    uint32_t* reg_pc = find_address_by_debug_table(all_items, "pc");
    if (!reg_pc) reg_pc = find_address_by_debug_table(all_items, "PC");

    // Slå på bassynken för T65-portarna direkt
    top.p_enable.set<bool>(true);
    top.p_rdy.set<bool>(true);
    top.p_so__n.set<bool>(true);
    top.p_mode.set<uint8_t>(0);

    if (!file_exists("oric_registers.snapshot")) {
        std::cout << "--- KÖRNING 1: STARTAR REN KALLSTART ---" << std::endl;
        
        std::vector<uint8_t> demo_code = { 
            0xEA,              // $1000: NOP
            0xE6, 0x00,        // $1001: INC $00
            0x4C, 0x01, 0x10   // $1003: JMP $1001
        };
        
        inject_program(oric_memory.data(), 0x1000, demo_code);

        // Hårdvarureset
        top.p_res__n.set<bool>(false);
        top.p_clk.set<bool>(false); top.step();
        top.p_clk.set<bool>(true);  top.step();
        top.p_res__n.set<bool>(true); 

        for (size_t c = 0; c < kallstart_cycles; ++c) {
            top.p_clk.set<bool>(false); top.step();
            drive_memory_bus(top); 
            top.p_clk.set<bool>(true);  top.step();

            printf("[CYKEL %3zu] PC=0x%.4X | RAM[$00]=0x%.2X\n", c, *reg_pc & 0xFFFF, oric_memory[0x0000]);
        }

        save_system_snapshot(top, oric_memory.data(), oric_memory.size() * sizeof(uint8_t), use_external_ram);
        printf("[SNAP] Sparat framgångsrikt vid PC=0x%.4X | RAM[$00]=0x%.2X\n", *reg_pc & 0xFFFF, oric_memory[0x0000]);
        std::cout << "Kör programmet EN GÅNG TILL för att testa återstarten!\n";
        return 0;

    } else {
        std::cout << "--- KÖRNING 2: DETEKTERADE SNAPSHOT, ÅTERSTARTAR (UTAN VARMSTART) ---" << std::endl;
        std::fill(oric_memory.begin(), oric_memory.end(), 0);

        // VARMSTART BORTTAGEN! 
        // Vi kör INGA reset-cykler alls nu på den nya färska processorn.
        // Vi dundrar in snapshoten DIREKT från disk i det absolut första vi gör.
        if (load_system_snapshot(top, oric_memory.data(), oric_memory.size() * sizeof(uint8_t), use_external_ram)) {
            
            // Port-gips efter restore
            top.p_enable.set<bool>(true);
            top.p_rdy.set<bool>(true);
            top.p_so__n.set<bool>(true);
            top.p_mode.set<uint8_t>(0);
            top.p_res__n.set<bool>(true); 

            printf("[LOAD] Återställd till PC=0x%.4X | RAM[$00]=0x%.2X\n", *reg_pc & 0xFFFF, oric_memory[0x0000]);

            std::cout << "[TEST] Tickar vidare från laddat tillstånd..." << std::endl;
            for (int c = 1; c <= 20; ++c) {
                top.p_clk.set<bool>(false); top.step();
                drive_memory_bus(top);
                top.p_clk.set<bool>(true);  top.step();

                printf("  [STEG %2d] PC=0x%.4X | RAM[$00]=0x%.2X\n", c, *reg_pc & 0xFFFF, oric_memory[0x0000]);
            }
            std::cout << "--- SIMULERING SLUTFÖRD ---" << std::endl;
        }
        return 0;
    }
}

