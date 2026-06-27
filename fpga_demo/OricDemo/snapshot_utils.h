#ifndef SNAPSHOT_UTILS_H
#define SNAPSHOT_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <cxxrtl/cxxrtl.h>

// Inkludera din rena, statiska simulator-header (genererad utan generics)
#include "cpu_ula_sim.h" 

// CENTRALISERING: Det rena statiska namnet för din toppnivå utan hashar!
using OricDesign = cxxrtl_design::p_oricatmostop;

// Centraliserad åtkomst till CPU-cellen
inline auto& get_cpu(OricDesign &top) {
    return top.cell_p_oric.cell_p_inst__cpu; 
}

// Centraliserad åtkomst till ULA-cellen
inline auto& get_ula(OricDesign &top) {
    return top.cell_p_oric.cell_p_inst__ula; 
}

// Centraliserad masterklocka (Skyddar mot p_clk__in / p_clk_in variationer)
inline void set_clock(OricDesign &top, bool value) {
    // Använder den faktiska masterklockan som identifierats i oric_sim_inject.cpp
    top.p_CLK__24MHz.set(value); 
}

// Standardfunktionerna för vår bevisade snapshot-motor
bool verify_snapshot_presence(const std::string& ram_file, const std::string& reg_file);
void save_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram=true);
bool load_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram=true);
uint32_t* find_address_by_debug_table(cxxrtl::debug_items& items, std::string key);
void inject_program(uint8_t* memory_array, uint16_t start_address, const std::vector<uint8_t>& machine_code);

void load_real_atmos_rom(uint8_t* ram, const char*);

#endif // SNAPSHOT_UTILS_H
