#ifndef RETRO_MONITOR_H
#define RETRO_MONITOR_H

#include <cstdint>
#include <cxxrtl/cxxrtl.h> // Krävs för cxxrtl::debug_items

namespace RetroMonitor {
    // ANROP 1: Helt tom på port-pekare! Monitorn jagar rätt på allt själv via debug_items.
    void init(cxxrtl::debug_items& items, uint16_t target_pc, size_t history_depth = 64);

    // ANROP 2: Körs varje klockcykel (helt oförändrad)
    bool tick(uint64_t current_cycle, const uint8_t* ram);

    void disable();
}

#endif // RETRO_MONITOR_H

