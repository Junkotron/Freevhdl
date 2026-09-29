#ifndef ASMTEST_HELPER_HPP
#define ASMTEST_HELPER_HPP

#include "../oric_rig.hpp"
#include <iostream>
#include <fstream>
#include <string>

class AsmTestRunner {
public:
    OricRig rig;

    // Startar riggen, slår på externt ROM och laddar in binärfilen
    bool setup(const std::string& bin_filename, uint16_t load_addr = 0xC000) {
        
        // =====================================================================
        // 🔍 STENHÅRD BESIKTNING AV FILENS RESET-VEKTOR DIREKT FRÅN DISK
        // =====================================================================
        std::ifstream file(bin_filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "❌ [TEST CRITICAL] Hittade inte binärfilen: " << bin_filename << std::endl;
            return false;
        }

        std::streamsize file_size = file.tellg();
        if (file_size < 16384) {
            std::cout << "\n⚠️  [HUMBLE WARNING] Testfilen \"" << bin_filename << "\" är bara " << std::dec << file_size << " bytes." << std::endl;
            std::cout << "💀 Den är för kort för att nå upp till Reset-vektorn ($FFFC-$FFFF)!" << std::endl;
            std::cout << "👉 Är du säker på att du vill fortsätta köra detta wasteland? (Y/N): " << std::flush;
            char ch; std::cin >> ch;
            if (ch != 'Y' && ch != 'y') {
                std::cout << "🛑 Avbryter omedelbart." << std::endl;
                return false;
            }
        } else {
            // Filen är fullängd (16KB) – då läser vi ut de exakta vektor-bytesen längst bak
            file.seekg(16380, std::ios::beg);
            uint8_t lsb = 0, msb = 0;
            file.read(reinterpret_cast<char*>(&lsb), 1);
            file.read(reinterpret_cast<char*>(&msb), 1);
            uint16_t vector_addr = (msb << 8) | lsb;

            if (vector_addr == 0x0000 || vector_addr == 0xEAEA) {
                std::cout << "\n⚠️  [HUMBLE WARNING] Reset-vektorn i filen pekar på $" 
                          << std::hex << std::uppercase << vector_addr << std::endl;
                std::cout << "💀 Hårdvaran kommer att kraschlanda rakt ner på adress noll/wasteland vid boot!" << std::endl;
                std::cout << "👉 Är du säker på att du vill fortsätta köra detta wasteland? (Y/N): " << std::flush;
                char ch; std::cin >> ch;
                if (ch != 'Y' && ch != 'y') {
                    std::cout << "🛑 Avbryter omedelbart." << std::endl;
                    return false;
                }
                std::cout << "🤠 Sadel på! Vi rider ut i det digitala ödelandet..." << std::endl;
            } else {
                std::cout << "🎯 [SANITY OK] Reset-vektorn i \"" << bin_filename << "\" är godkänd: Pekar på $" 
                          << std::hex << std::uppercase << vector_addr << std::dec << std::endl;
            }
        }
        file.close();

        // =====================================================================
        // HÅRDVARUSTART (När filen är godkänd)
        // =====================================================================
        rig.set_rom_select(1); // [5, 6] Alltid externt ROM under dessa tester

        if (!rig.load_rom_image(bin_filename, load_addr)) { // [5, 6]
            std::cerr << "❌ [TEST CRITICAL] Kunde inte ladda: " << bin_filename << std::endl; // [5, 6]
            return false; // [5, 6]
        } // [5, 6]

        // Kör hårdvaru-reset så att T65-blocket vaknar ordentligt
        rig.hardware_reset(); // [5, 6]
        
        std::cout << "🚀 [SIM] CPU kickad och redo för exekvering!" << std::endl;
        return true;
    }

    // Kör en cykel (High + Low)
    void tick() { // [5, 6]
        rig.clock_master_high(); // [5, 6]
        rig.clock_master_low(); // [5, 6]
    } // [5, 6]
};

#endif // ASMTEST_HELPER_HPP
