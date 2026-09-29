#include "asmtest_helper.hpp"
#include <iostream>

int main() {
    AsmTestRunner test;

    if (!test.setup("asmtest/via_register_test.bin")) {
        return 1;
    }

    bool fann_55 = false;
    bool fann_aa = false;

    // Kör 1000 mastercykler i det tysta för att CPU/VIA ska hinna läsa/skriva
    for (int cycle = 0; cycle < 1000; cycle++) {
        test.tick();

        uint8_t bus_data = test.rig.get_bus_data();
        bool is_write = test.rig.get_we();

        // Vi spionerar passivt efter våra mönster vid läscykler
        if (!is_write) {
            if (bus_data == 0x55) fann_55 = true;
            if (bus_data == 0xAA) fann_aa = true;
        }
    }

    // 🏁 Den enkla, rena slutdomen
    if (fann_55 && fann_aa) {
        std::cout << "✨ [PASSED] via_register_test: Statisk R/W mot VIA fungerar perfekt!" << std::endl;
        return 0;
    } else {
        std::cerr << "❌ [FAILED] via_register_test: Registren svarade inte med rätt mönster!" << std::endl;
        if (!fann_55) std::cerr << "   -> Saknade monster $55 på bussen." << std::endl;
        if (!fann_aa) std::cerr << "   -> Saknade monster $AA på bussen." << std::endl;
        return 1;
    }
}
