#ifndef SNAPSHOT_UTILS_H
#define SNAPSHOT_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <cxxrtl/cxxrtl.h>

// Inkludera din rena, statiska simulator-header (genererad utan generics)
#include "cpu_ula_sim.h" 

// CENTRALISERING: Det rena statiska namnet för din toppnivå utan hashar!
using OricDesign = cxxrtl_design::p_oricatmos__Brtl;

// Centraliserad åtkomst till CPU-cellen
inline auto& get_cpu(OricDesign &top) {
    return top.cell_t65_Brtl; 
}

// Centraliserad åtkomst till ULA-cellen
inline auto& get_ula(OricDesign &top) {
    return top.cell_ula_Brtl; 
}

// Centraliserad masterklocka (Skyddar mot p_clk__in / p_clk_in variationer)
inline void set_clock(OricDesign &top, bool value) {
    // Statisk Yosys-export använder oftast ett enkelt understreck för topp-portar.
    // Skulle din kompilator klaga på just denna rad, ändra till p_clk__in (dubbelt)!
    top.p_clk_in.set<bool>(value); 
}

// Standardfunktionerna för vår bevisade snapshot-motor
void save_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram);
bool load_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram);
uint32_t* find_address_by_debug_table(cxxrtl::debug_items& items, std::string key);
void inject_program(uint8_t* memory_array, uint16_t start_address, const std::vector<uint8_t>& machine_code);

#endif // SNAPSHOT_UTILS_H
