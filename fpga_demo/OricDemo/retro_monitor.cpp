#include "retro_monitor.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>    // Säkrar std::string för sökningar
#include <cstdlib>   // Säkrar std::exit() för den feta exiten
#include <algorithm> // För framtida strängmanipuleringar

namespace RetroMonitor {

    struct BusSnapshot {
        uint64_t cycle;
        uint16_t addr;
        uint8_t  data;
        bool     rw;
    };

    // Interna tillstånd och svans-inställningar
    static std::vector<BusSnapshot> flight_recorder;
    static size_t max_depth = 64;
    static uint16_t trigger_pc = 0x0000;
    static bool is_active = true;

    // Interna råa CXXRTL-pekare (Typade till uint32_t* för att matcha hårdvaran)
    static uint32_t* cxx_addr     = nullptr;
    static uint32_t* cxx_rw       = nullptr;
    static uint32_t* cxx_data_out = nullptr;

    // Snygg privat helper för din legendariska 1987-layout
    static void print_dump(uint16_t current_pc, const uint8_t* ram) {
        std::cout << "\n================================================================\n";
        std::cout << " 💾 ORIC ATMOS MACHINE CODE MONITOR v1.2 (1987) \n";
        std::cout << "================================================================\n";
        
        std::cout << "--- [FLIGHT RECORDER: LAST PROBE CYCLES] -----------------------\n";
        std::cout << " CYCL | ADDR  | DATA | R/W |\n";
        std::cout << "------+-------+------+-----+\n";
        for (const auto& snap : flight_recorder) {
            std::cout << " " << std::setw(4) << std::setfill('0') << snap.cycle << " | "
                      << "$" << std::hex << std::setw(4) << snap.addr << " | "
                      << "$" << std::setw(2) << (int)snap.data << "  | "
                      << (snap.rw ? "RD" : "WR") << "  |\n";
        }
        
        std::cout << "\n--- [MEMORY MONITOR DUMP] --------------------------------------\n";
        uint32_t aligned_start = (current_pc & 0xFFF0);
        uint16_t start_dump = (aligned_start >= 0x20) ? (aligned_start - 0x20) : 0x0000;
        
        for (int row = 0; row < 6; row++) {
            uint16_t row_addr = start_dump + (row * 8);
            std::cout << "$" << std::hex << std::setw(4) << std::setfill('0') << row_addr << ": ";
            
            for (int i = 0; i < 8; i++) {
                std::cout << std::setw(2) << (int)ram[(uint16_t)(row_addr + i)] << " ";
            }
            std::cout << " | ";
            for (int i = 0; i < 8; i++) {
                char c = ram[(uint16_t)(row_addr + i)];
                std::cout << ((c >= 32 && c <= 126) ? c : '.');
            }
            if (current_pc >= row_addr && current_pc < row_addr + 8) {
                std::cout << "  <-- CURRENT PC ($" << std::hex << current_pc << ")";
            }
            std::cout << "\n";
        }
        std::cout << "================================================================\n\n";
    }


    void init(cxxrtl::debug_items& items, uint16_t target_pc, size_t history_depth) {
        trigger_pc   = target_pc;
        max_depth    = history_depth;
        is_active    = true;
        
        flight_recorder.clear();
        flight_recorder.reserve(max_depth);

        // Din sökfunktion
        auto lookup = [&](const std::string& key) -> uint32_t* {
            auto it = items.table.find(key);
            if (it != items.table.end() && !it->second.empty()) {
                return it->second.front().curr;
            }
            return nullptr;
        };

        // Koppla direkt mot de exakta strängarna från din röntgen
        cxx_addr     = lookup("oric inst_ula addr");
        cxx_rw       = lookup("oric cpu_rw");
        cxx_data_out = lookup("oric inst_ula ldatabus");

        // Om det skiter sig, dumpa alternativen och gör en FET EXIT!
        if (!cxx_addr || !cxx_rw || !cxx_data_out) {
            std::cerr << "\n🔥 [RETRO MONITOR CRITICAL ERROR] Kunde inte hitta hårdvaruportarna i CXXRTL-tabellen!\n";
            std::cerr << "--- [RÖNTGEN: MATCHANDE NYCKLAR I DIN DESIGN] -----------------\n";
            
            bool found_any = false;
            for (auto const& [name, value] : items.table) {
                if (name.find("addr") != std::string::npos || 
                    name.find("rw") != std::string::npos || 
                    name.find("data") != std::string::npos) {
                    std::cerr << "  👉  \"" << name << "\"\n";
                    found_any = true;
                }
            }
            std::cerr << "--------------------------------------------------------------\n";
            std::exit(1);
        } 
        
        std::cout << "🎯 [RETRO MONITOR] Säkrat alla hårdvarupekare autonomt via oric-hierarkin!\n";
    }

    bool tick(uint64_t current_cycle, const uint8_t* ram) {
        if (!is_active || !cxx_addr || !cxx_rw || !cxx_data_out) return false;

        // Läs av hårdvarun via de sparade uint32_t*-pekarna
        uint16_t addr = static_cast<uint16_t>(*cxx_addr);
        bool rw       = static_cast<bool>(*cxx_rw);
        uint8_t data  = rw ? ram[addr] : static_cast<uint8_t>(*cxx_data_out);

        // Sopa svansen automatiskt
        if (flight_recorder.size() >= max_depth) {
            flight_recorder.erase(flight_recorder.begin());
        }
        flight_recorder.push_back({current_cycle, addr, data, rw});

        // Detektera om CPU har landat på din magiska $F5CD
        if (addr == trigger_pc) {
            is_active = false;
            print_dump(addr, ram);
            return true; // Smäll av och meddela main()
        }

        return false;
    }

    void disable() {
        is_active = false;
        flight_recorder.clear();
    }
}

