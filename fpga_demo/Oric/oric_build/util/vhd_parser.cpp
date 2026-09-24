
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>  // 💡 DENNA SAKNADES! Fixar uint8_t och uint16_t för gamla g++


int main(int argc, char** argv) {
    std::string filename = "BASIC11.vhd";
    if (argc > 1) {
        filename = argv[1];
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Kunde inte öppna VHDL-filen: " << filename << std::endl;
        return 1;
    }

    std::vector<uint8_t> rom_bytes;
    std::string line;

    while (std::getline(file, line)) {
        size_t pos = 0;
        // Leta efter X eller x genom hela raden
        while ((pos = line.find_first_of("xX", pos)) != std::string::npos) {
            // Kontrollera att det är ett mönster av typen X"HH"
            if (pos + 3 < line.length() && line[pos + 1] == '"' && line[pos + 4] == '"') {
                std::string hex_pair = line.substr(pos + 2, 2);
                
                // Konvertera strängen till en rå byte
                unsigned int byte_val;
                std::stringstream ss;
                ss << std::hex << hex_pair;
                if (ss >> byte_val) {
                    rom_bytes.push_back(static_cast<uint8_t>(byte_val));
                }
                pos += 5; // Hoppa förbi hela blocket X"HH"
            } else {
                pos++;
            }
        }
    }
    file.close();

    std::cout << "✅ KMV PARSER SUCCÉ!" << std::endl;
    std::cout << "Hittade totalt: " << std::dec << rom_bytes.size() << " bytes maskinkod." << std::endl;

    if (rom_bytes.size() != 16384) {
        std::cout << "⚠️ Varning: Matrisen innehåller inte exakt 16KB (16383 index)! Hittade: " << rom_bytes.size() << std::endl;
    }

    // Skriv ut de första och sista bytesen som en snabb sanity-check
    if (!rom_bytes.empty()) {
        std::cout << "\n--- SANITY CHECK (Första 16 bytes) ---";
        for (size_t i = 0; i < 16 && i < rom_bytes.size(); ++i) {
            if (i % 8 == 0) std::cout << "\n";
            std::cout << "X\"" << std::hex << std::setw(2) << std::setfill('0') << (int)rom_bytes[i] << "\" ";
        }
        std::cout << "\n\n--- VESTOR-KONTROLL (Sista 4 bytes) ---";
        std::cout << "\nReset-vektor (ska peka på startadressen): ";
        if (rom_bytes.size() >= 16384) {
            // Oricens ROM ligger på $C000-$FFFF. Vektorerna ligger på $FFFA-$FFFF.
            // Inuti en 16KB array hamnar $FFFC och $FFFD på index 16380 och 16381.
            uint16_t reset_vector = rom_bytes[16380] | (rom_bytes[16381] << 8);
            std::cout << "$" << std::hex << std::setw(4) << std::setfill('0') << reset_vector << std::endl;
        } else {
            std::cout << "Filen var för kort för att läsa vektorer." << std::endl;
        }
    }

    // 💾 Spara som en ren binärfil (BASIC11A.rom) så att vi har den på disken!
    std::ofstream out_bin("BASIC11A.rom", std::ios::binary);
    if (out_bin.is_open()) {
        out_bin.write(reinterpret_cast<const char*>(rom_bytes.data()), rom_bytes.size());
        out_bin.close();
        std::cout << "\n💾 Sparade ren binärfil till: BASIC11A.rom" << std::endl;
    }

    return 0;
}

