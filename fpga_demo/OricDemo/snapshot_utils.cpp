#include "snapshot_utils.h"
#include <iostream>
#include <fstream>
#include <vector>


// En ren mjukvaru-rutin som bara kollar om dina snapshot-filer faktiskt finns på disken
bool verify_snapshot_presence(const std::string& ram_file, const std::string& reg_file) {
    std::ifstream ram_chk(ram_file, std::ios::binary);
    std::ifstream reg_chk(reg_file, std::ios::binary);
    
    // Om båda filerna går att öppna betyder det att de finns och är redo!
    if (ram_chk.good() && reg_chk.good()) {
        ram_chk.close();
        reg_chk.close();
        return true;
    }
    
    return false;
}


void load_real_atmos_rom(uint8_t* ram, const char* fname) {
    // Öppna Atmos ROM (oftast 16KB, t.ex. BASIC 1.1b)
    std::ifstream rom_file(fname, std::ios::binary);
    
    if (!rom_file) {
        std::cerr << "⚠️ [WARN] Kunde inte hitta basic11b.rom! Kör vidare utan skarpt ROM.\n";
        return;
    }
    
    // Läs in 16384 bytes direkt till adress $C000 och framåt
    rom_file.read(reinterpret_cast<char*>(&ram[0xC000]), 16384);
    std::cout << "💾 [ROM] Atmos-standard ROM framgångsrikt laddat till $C000-$FFFF!\n";
}


// ============================================================================
// SPARFUNKTION (FILTERLÖS)
// ============================================================================
void save_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram) {
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

    for (auto &item : items.table) {
        std::string name = item.first;
        size_t idx = 0;

        for (auto &flag : item.second) {
            size_t current_idx = idx++; // Öka index säkert direkt för vektorn

            // HÄR FILTRERAR VI BORT ALLT SKRÄP FÖR GOTT:
            if (flag.type != 0) continue; 
            if (flag.curr == nullptr || flag.width == 0) continue;
            
            // Blockera feting-ROM:et och dolda lookup-tabeller baserat på storlek och namn
            if (flag.width > 4) continue;
            if (name.find("rom") != std::string::npos || name.find("ROM") != std::string::npos) continue;
            
            // Blockera kombinatoriska nät (flagga 8)
            if (flag.flags & (1 << 3)) continue; 

            // Det som överlever hit är enbart de små, äkta klockade hårdvaruregistren!
            uint32_t name_len = static_cast<uint32_t>(name.size());
            reg_os.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
            reg_os.write(name.data(), name_len);
            
            uint32_t saved_idx = static_cast<uint32_t>(current_idx);
            reg_os.write(reinterpret_cast<const char*>(&saved_idx), sizeof(saved_idx));

            size_t bytes_to_write = flag.width * sizeof(cxxrtl::chunk_t);
            reg_os.write(reinterpret_cast<const char*>(flag.curr), bytes_to_write);
        }
    }
    reg_os.close();
    std::cout << "[SNAPSHOT] Register sparade stabilt!" << std::endl;
}

// ============================================================================
// INLÄSNINGSFUNKTION (TILLSTÅNDSSKYDDAD & ROM-IMMUN)
// ============================================================================
bool load_system_snapshot(OricDesign &top, void* my_ram, size_t ram_size, bool use_ram) {
    // 1. Återställ ditt externa C++ RAM-minne (oric_ram.bin)
    std::ifstream ram_is("oric_ram.bin", std::ios::binary);
    if (ram_is.is_open()) {
        ram_is.read(reinterpret_cast<char*>(my_ram), ram_size);
        ram_is.close();
        std::cout << "[SNAPSHOT] oric_ram.bin inläst och återställd." << std::endl;
    } else {
        std::cout << "[SNAPSHOT] Avbryter: Kunde inte hitta oric_ram.bin (normalt vid kallstart!" << std::endl;
        return false;
    }

    // 2. Öppna register-snapshoten
    std::ifstream reg_is("oric_registers.snapshot", std::ios::binary);
    if (!reg_is.is_open()) {
        std::cerr << "[SNAPSHOT-FEL] Avbryter: Kunde inte hitta oric_registers.snapshot!" << std::endl;
        return false;
    }

    // Hämta aktuella debug_items och scopes från din design
    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    size_t signals_loaded = 0;

    // Läs binärfilen linjärt tills den tar slut
    while (reg_is.peek() != EOF) {
        // A. LÄS METADATA (Exakt samma ordning som de sparades)
        uint32_t name_len = 0;
        reg_is.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        
        std::string name(name_len, '\0');
        reg_is.read(&name[0], name_len); // Säkert skriv till strängens interna buffert

        uint32_t saved_idx = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_idx), sizeof(saved_idx));

        // B. SLÅ UPP UTIFRÅN STRÄNGNAMNET DIREKT
        auto it = items.table.find(name);
        if (it != items.table.end() && saved_idx < it->second.size()) {

            auto &flag = it->second[saved_idx];
            size_t bytes_to_read = flag.width * sizeof(cxxrtl::chunk_t);

            // C. DEN SÄKRA FILSTRÖMS-HÄMTNINGEN
            // Vi läser ALLTID data direkt från disk till signalens minnescell
            // för att garantera att filpekaren flyttar sig exakt rätt antal bytes.
            reg_is.read(reinterpret_cast<char*>(flag.curr), bytes_to_read);

            // D. FILTER EFTER INLÄSNING (Rör ALDRIG .next för dolda eller farliga nät!)
            // Om signalen råkar ha en trasig pekare, är kombinatorisk (flagga 8),
            // eller tillhör det feta ROM-blocket, så skippar vi att röra registerskuggan.
            if (flag.curr == nullptr || flag.width == 0 || flag.width > 4 ||
                name.find("rom") != std::string::npos || name.find("ROM") != std::string::npos ||
                (flag.flags & (1 << 3))) 
            {
                // Signalen är fredad på hårdvarunivå. Vi går tryggt vidare till nästa rad i filen.
                continue; 
            }

            // E. REGISTERSYNK (Enbart för äkta, rörliga register/flip-flops)
            // Tvinga registerskuggan (.next) att ta samma värde som vi precis laddade.
            if (flag.next && flag.next != flag.curr) {
                for (size_t i = 0; i < flag.width; ++i) {
                    flag.next[i] = flag.curr[i];
                }
            }
            signals_loaded++;
        } else {
            // Om filen innehåller en signal som strukits i RTL, har vi ett allvarligt synkfel
            std::cerr << "[SNAPSHOT-FEL] Desynk! Hittade sparad signal som saknas i nuvarande RTL: " << name << std::endl;
            reg_is.close();
            return false;
        }
    }
    reg_is.close();
    
    // 3. ÅTERSTÄLL ALLA KOMBINATORISKA NÄT NATURLIGT
    // Nu när alla register (flip-flops) har fått sina korrekta värden, 
    // låter vi CXXRTL räkna ut alla klockor och ledningar (som s_tape_byte_enable)
    top.eval(); 
    
    std::cout << "[SNAPSHOT] Återställning klar! Laddade " << signals_loaded << " äkta register framgångsrikt." << std::endl;
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

