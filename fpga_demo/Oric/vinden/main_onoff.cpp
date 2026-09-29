#include "snapshot_utils.h"
#include <iostream>
#include <fstream>
#include <vector>

// ============================================================================
// SPARFUNKTION (FILTERLÖS)
// ============================================================================
void save_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram) {
    if (use_ram && my_ram && ram_size > 0) {
        std::ofstream ram_os("oric_ram.bin", std::ios::binary);
        if (ram_os.is_open()) {
            ram_os.write(reinterpret_cast<const char*>(my_ram), ram_size);
            ram_os.close();
        }
    }

    std::ofstream reg_os("oric_registers.snapshot", std::ios::binary);
    if (!reg_os.is_open()) return;

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    const size_t bits_per_chunk = sizeof(cxxrtl::chunk_t) * 8;
    size_t saved_counter = 0;

    for (auto &item : items.table) {
        std::string name = item.first;
        size_t idx = 0;

        for (auto &flag : item.second) {
            size_t current_idx = idx++; 

            if (flag.curr == nullptr || flag.width == 0) continue;

            size_t chunks_needed = (flag.width + bits_per_chunk - 1) / bits_per_chunk;
            size_t bytes_to_write = chunks_needed * sizeof(cxxrtl::chunk_t);

            uint32_t name_len = static_cast<uint32_t>(name.size());
            reg_os.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
            reg_os.write(name.data(), name_len);
            
            uint32_t saved_idx = static_cast<uint32_t>(current_idx);
            reg_os.write(reinterpret_cast<const char*>(&saved_idx), sizeof(saved_idx));

            uint32_t saved_chunks = static_cast<uint32_t>(chunks_needed);
            reg_os.write(reinterpret_cast<const char*>(&saved_chunks), sizeof(saved_chunks));

            std::vector<cxxrtl::chunk_t> local_buffer(chunks_needed);
            for (size_t i = 0; i < chunks_needed; ++i) {
                local_buffer[i] = flag.curr[i];
            }
            reg_os.write(reinterpret_cast<const char*>(local_buffer.data()), bytes_to_write);
            saved_counter++;
        }
    }
    reg_os.close();
    std::cout << "[SNAPSHOT] Klart! Sparade " << saved_counter << " nät till disk." << std::endl;
}

// ============================================================================
// INLÄSNINGSFUNKTION (TILLSTÅNDSSKYDDAD & ROM-IMMUN)
// ============================================================================
bool load_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram) {
    if (use_ram) {
        std::ifstream ram_is("oric_ram.bin", std::ios::binary);
        if (ram_is.is_open()) {
            ram_is.read(reinterpret_cast<char*>(my_ram), ram_size);
            ram_is.close();
        }
    }

    std::ifstream reg_is("oric_registers.snapshot", std::ios::binary);
    if (!reg_is.is_open()) return false;

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    size_t signals_loaded = 0;
    const size_t bits_per_chunk = sizeof(cxxrtl::chunk_t) * 8;

    while (reg_is.peek() != EOF) {
        uint32_t name_len = 0;
        reg_is.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        
        std::string name(name_len, '\0');
        reg_is.read(name.data(), name_len); // Säkrad GCC 15-buffertläsning via .data() [cxxrtl]

        uint32_t saved_idx = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_idx), sizeof(saved_idx));

        uint32_t saved_chunks = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_chunks), sizeof(saved_chunks));

        size_t bytes_to_read = saved_chunks * sizeof(cxxrtl::chunk_t);

        std::vector<cxxrtl::chunk_t> temporary_buffer(saved_chunks);
        reg_is.read(reinterpret_cast<char*>(temporary_buffer.data()), bytes_to_read);

        // STENHÅRD BRANDVÄGG MOT STATISKA ROM-MATRISER (Krossar SegFaulten) [cxxrtl]
        if (name.find("rom") != std::string::npos) {
            continue;
        }

        auto it = items.table.find(name);
        if (it != items.table.end() && saved_idx < it->second.size()) {
            auto &flag = it->second[saved_idx];
            size_t current_chunks = (flag.width + bits_per_chunk - 1) / bits_per_chunk;

            if (flag.curr != nullptr && current_chunks == saved_chunks) {
                // Skriv endast om nätet är adresserbart och modifierbart (type == 0) [cxxrtl]
                if (flag.type == 0) {
                    for (size_t i = 0; i < current_chunks; ++i) {
                        flag.curr[i] = temporary_buffer[i];
                    }

                    if (flag.next && flag.next != flag.curr) {
                        for (size_t i = 0; i < current_chunks; ++i) {
                            flag.next[i] = temporary_buffer[i];
                        }
                    }
                    signals_loaded++;
                }
            }
        }
    }
    reg_is.close();
    top.eval(); // Synka om exekveringsgrafen stabilt efter restore [cxxrtl]
    std::cout << "[SNAPSHOT] Återställning klar! Laddade säkert in " << signals_loaded << " rörliga register." << std::endl;
    return true;
}

// ============================================================================
// UTILITIES
// ============================================================================
uint32_t* find_address_by_debug_table(cxxrtl::debug_items& items, std::string key) {
    auto it = items.table.find(key);
    if (it != items.table.end()) {
        return it->second.front().curr;
    }
    return nullptr;
}

void inject_program(uint8_t* memory_array, uint16_t start_address, const std::vector<uint8_t>& machine_code) {
    for (size_t i = 0; i < machine_code.size(); ++i) {
        if ((start_address + i) < 65536) {
            memory_array[start_address + i] = machine_code[i];
        }
    }
    memory_array[0xFFFC] = start_address & 0xFF;        
    memory_array[0xFFFD] = (start_address >> 8) & 0xFF; 
}

