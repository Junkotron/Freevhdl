#ifndef ORIC_RIG_HPP
#define ORIC_RIG_HPP

#include <string>
#include <cstdint>
#include <fstream>

// 🕵️‍♂️ Den Trojanska Framåtrekorden – Döljer hela CXXRTL-strukturen för main.cpp
struct OricContext;

class OricRig {
private:
    OricContext* ctx; // Vår dolda mjukvarubrygga till nätlistan

public:
    // Konstruktor och destruktor (Måste definieras i .cpp-filen!)
    OricRig();
    ~OricRig();

    // Styrsignaler (Kallstart och klockträd)
    void hardware_reset();
    void clock_master_high();
    void clock_master_low();

    // 💡 Sätter ROM-konfigurationen (t.ex. 0b11 för BASIC-ROM)
    void set_rom_select(uint8_t mode);
  
    // Spionsignaler (Mappers till dina råa VHDL-portar)
    uint16_t get_pc() const;
    uint8_t  get_bus_data() const;
    uint64_t get_cpu_regs() const;
    bool     get_we() const;
    bool     get_cpu_clk() const;
    bool     is_sync() const;
    bool     get_via_irq() const; // 🕵️‍♂️ NY: Avbrottsdetektorn!
  
    // utilities
    bool load_rom_image(const std::string& filename, uint16_t start_addr);

};

#endif
