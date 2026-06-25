// example compile with :
// g++ -g -I `yosys-config  --datdir`/include/backends/cxxrtl/runtime maintest.cpp Oric_MiSTer/rtl/oricatmos_sim.o

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cxxrtl/cxxrtl.h>
#include <string>

using namespace std;

uint32_t* find_address_by_debug_table(cxxrtl::debug_items& items, string key)
{

  cout << "Searching for signal: " << key << endl;
  
  // 1. Sök efter adressbussen i samma tabell
  auto it = items.table.find(key); // Justera namnet till er signal

  if (it != items.table.end()) {
    const cxxrtl::debug_item &addr_item = it->second.front();

    // 2. För en vanlig signal (wire) pekar .curr direkt på råa uint32_t-block
    // Vi behöver inte göra en reinterpret_cast till en hel array, bara läsa första blocket
    uint32_t *raw_data = addr_item.curr;
    return raw_data;
  }
  return nullptr;
}


uint8_t memory_array[65536];

// Ersätt med den fil som faktiskt drar in din "oricatmos_sim.h"
#include "Oric_MiSTer/rtl/oricatmos_sim.h" 

void save_system_snapshot(cxxrtl_design::p_oricatmostop__sim &top, void* my_ram, size_t ram_size) {
    std::ofstream ram_os("oric_ram.bin", std::ios::binary);
    if (ram_os.is_open()) {
        ram_os.write(reinterpret_cast<const char*>(my_ram), ram_size);
        ram_os.close();
    }

    std::ofstream reg_os("oric_registers.snapshot", std::ios::binary);
    if (!reg_os.is_open()) return;

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    const size_t bits_per_chunk = sizeof(cxxrtl::chunk_t) * 8;

    for (auto &item : items.table) {
        std::string name = item.first;
        size_t idx = 0;

        for (auto &flag : item.second) {
            size_t current_idx = idx++; 

            if (flag.curr == nullptr || flag.width == 0) continue;
            if (flag.type != 0) continue; // Hoppa över konstanta/skrivskyddade nät helt

            if (name.find("rom") != std::string::npos || name.find("ROM") != std::string::npos) continue;
            if (name.find("ram") != std::string::npos || name.find("RAM") != std::string::npos) {
                if (flag.width > 16) continue; 
            }

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
        }
    }
    reg_os.close();
    std::cout << "[SNAPSHOT] Register sparade utan skrivskyddade element!" << std::endl;
}
bool load_system_snapshot(cxxrtl_design::p_oricatmostop__sim &top, void* my_ram, size_t ram_size) {
    std::ifstream ram_is("oric_ram.bin", std::ios::binary);
    if (ram_is.is_open()) {
        ram_is.read(reinterpret_cast<char*>(my_ram), ram_size);
        ram_is.close();
        std::cout << "[SNAPSHOT] oric_ram.bin inläst och återställd." << std::endl;
    } else {
        std::cerr << "[SNAPSHOT-FEL] Avbryter: Kunde inte hitta oric_ram.bin!" << std::endl;
        return false;
    }

    std::ifstream reg_is("oric_registers.snapshot", std::ios::binary);
    if (!reg_is.is_open()) {
        std::cerr << "[SNAPSHOT-FEL] Avbryter: Kunde inte hitta oric_registers.snapshot!" << std::endl;
        return false;
    }

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    size_t signals_loaded = 0;
    const size_t bits_per_chunk = sizeof(cxxrtl::chunk_t) * 8;

    while (reg_is.peek() != EOF) {
        uint32_t name_len = 0;
        reg_is.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        
        std::string name(name_len, '\0');
        // KORRIGERAT: Läs direkt till strängens interna char-array via &name[0]
        reg_is.read(&name[0], name_len); 

        uint32_t saved_idx = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_idx), sizeof(saved_idx));

        uint32_t saved_chunks = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_chunks), sizeof(saved_chunks));

        size_t bytes_to_read = saved_chunks * sizeof(cxxrtl::chunk_t);

        std::vector<cxxrtl::chunk_t> temporary_buffer(saved_chunks);
        reg_is.read(reinterpret_cast<char*>(temporary_buffer.data()), bytes_to_read);

        auto it = items.table.find(name);
        if (it != items.table.end() && saved_idx < it->second.size()) {
            auto &flag = it->second[saved_idx];
            size_t current_chunks = (flag.width + bits_per_chunk - 1) / bits_per_chunk;

            if (flag.curr != nullptr && current_chunks == saved_chunks && flag.type == 0) {
                
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
    reg_is.close();
    
    top.eval(); 
    
    std::cout << "[SNAPSHOT] Återställning klar! Laddade " << signals_loaded << " rörliga register." << std::endl;
    return true;
}


int main() {
  // Definiera S (ändra till det antal cykler du vill testa, t.ex. 1000)
  const size_t S = 1000; 

  cxxrtl_design::p_oricatmostop__sim top;
  cxxrtl::debug_items all_items;
  cxxrtl::debug_scopes scopes;

  top.debug_info(&all_items, &scopes, "");  
  uint32_t* cpu_addr = find_address_by_debug_table(all_items, "oric cpu_ad");
  assert(cpu_addr != nullptr);
    
  std::memset(memory_array, 0, sizeof(memory_array));

  std::cout << "--- STARTAR ORIC SIMULATOR ---" << std::endl;

  // Försök ladda snapshot från disk
  bool restarted = load_system_snapshot(top, memory_array, sizeof(memory_array));
    
  size_t start_cycle = 0;
  size_t end_cycle = S;

  if (!restarted) {
    std::cout << "[BOOT] Ingen snapshot hittades. Håller RESET hög..." << std::endl;
    top.p_RESET.set<uint8_t>(true);
    top.step(); 
  } else {
    std::cout << "[BOOT] Återstartad från snapshot! Fortsätter simulering från cykel " << S << "..." << std::endl;
    start_cycle = S;

    // TODO cargo cult?
    top.p_CLK__24MHz.set<bool>(false);
    top.eval(); 
  }

  // Simuleringsloop (Kör till 2*S om vi startade om, så vi får se nästa fas)
  size_t max_cycles = 2 * end_cycle;
  
  for (size_t cycle = start_cycle; cycle < max_cycles; ++cycle) {
    
    // Reset-sekvens hanteras ENBART vid en ren nyboot
    if (!restarted && cycle == 20) {
      std::cout << "[BOOT] Cykel 20 nådd. Släpper RESET..." << std::endl;
      top.p_RESET.set<uint8_t>(false);  
    }
    
    // Ett fullt klocktick på Orics 24MHz-klocka
    top.p_CLK__24MHz.set<bool>(false);
    top.step();
    top.p_CLK__24MHz.set<bool>(true);
    top.step();

    // Skriv ut aktuell adress som CPU:n tittar på
    printf("[CYKEL %zu] ADDR=%.4X\n", cycle, (*cpu_addr) & 0xffff);
    
    // Demotrigg: Spara snapshot vid cykel S om detta är en ren nyboot
    if (!restarted && cycle == S) {
      std::cout << "[TRIGGER] Cykel S (" << S << ") nådd! Sparar och avbryter för test..." << std::endl;
      save_system_snapshot(top, memory_array, sizeof(memory_array));
      std::cout << "Kör programmet en gång till för att verifiera automatisk load!" << std::endl;
      return 0; 
    }
  }

  std::cout << "--- SIMULERING SLUTFÖRD UTAN KRASCH ---" << std::endl;
  return 0;
}
