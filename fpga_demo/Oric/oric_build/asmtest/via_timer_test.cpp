#include "asmtest_helper.hpp"
#include <iostream>

int main() {
    AsmTestRunner test;

    if (!test.setup("asmtest/via_timer_test.bin")) {
        return 1;
    }

    bool irq_triggad = false;

    // Kör 20 000 mastercykler i total tystnad
    for (int cycle = 0; cycle < 20000; cycle++) {
        test.tick();

        if (test.rig.get_via_irq()) {
            irq_triggad = true;
            break;
        }
    }

    // 🏁 Slutdom
    if (irq_triggad) {
        std::cout << "✨ [PASSED] via_timer_test: Timer 1 genererade IRQ på kiselnivå!" << std::endl;
        return 0;
    } else {
        std::cerr << "❌ [FAILED] via_timer_test: Timern lyckades aldrig dra i IRQ-linan inom tidsramen!" << std::endl;
        return 1;
    }
}
