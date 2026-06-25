#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include "cpu_ula_sim.h" // Din fungerande hårdvaruheader

// Vi uppgraderar minnet till fulla 64K så att det täcker ALLA adresser (0x0000 - 0xFFFF)
uint8_t oric_ram[65536];

int main() {
    std::cout << "--- ORIC ATMOS HARDCORE MVP-TEST (64K ROM/RAM) ---" << std::endl;

    /*
    
    // 1. LÄS IN HELA DUMPEN (RAM + ROM) DIREKT I DIN ARRAY
    std::ifstream file("oric_ram.bin", std::ios::binary);
    if (!file.is_open()) {
        std::cout << "[FEL] Kunde inte öppna oric_ram.bin! Avbryter." << std::endl;
        return 1;
    }
    
    // Läs in hela filen (Om filen är 48K läser den 48K, om den är 64K täcker den allt)
    file.read(reinterpret_cast<char*>(oric_ram), sizeof(oric_ram));
    std::cout << "[SUCCESS] " << file.gcount() << " bytes förladdade i C++-minnet!" << std::endl;
    file.close();

    */
    
    // 2. Instansiera hårdvarumodellen
    cxxrtl_design::p_oricatmostop top;
    auto& cpu = top.cell_p_oric.cell_p_inst__cpu;

    // 3. KALLSTART: Låt hårdvaran cykla med RESET aktiv
    top.p_RESET.set<bool>(true);
    for (int i = 0; i < 32; i++) {
        top.p_CLK__24MHz.set<bool>(false); top.step();
	top.p_CLK__24MHz.set<bool>(true);  top.step();
    }
    //p_CLK__24MHz
    // Släpp hårdvarureseten
    top.p_RESET.set<bool>(false);
    top.p_CLK__24MHz.set<bool>(false); top.step();
    top.p_CLK__24MHz.set<bool>(true);  top.step();

    // 4. INJEKTIONEN
    std::cout << "-> Tvingar PC till 0xF5D5..." << std::endl;
    cpu.p_pc.set<uint16_t>(0xF5D5);     
    cpu.p_abc.set<uint16_t>(0x00);      
    cpu.p_p.set<uint16_t>(0x24);        
    cpu.p_s.set<uint16_t>(0xFD);        

    // Krossa dolda reset-tillstånd
    cpu.p_rstcycle.set<bool>(false); 
    cpu.p_irqcycle.set<bool>(false); 
    cpu.p_nmicycle.set<bool>(false); 
    cpu.p_mcycle.set<uint16_t>(0);   

    top.step();

    std::cout << "-> Startar simulering..." << std::endl;

    // 5. EXEKVERINGSLOOPPEN
    for (uint64_t cycle = 0; cycle < 5000; cycle++) {
        
        top.p_CLK__24MHz.set<bool>(false); 
        top.step();

        top.p_CLK__24MHz.set<bool>(true);  
        top.step(); 

        // Hämta adressbussen
        uint16_t addr = top.p_ram__ad__out.get<uint16_t>();
        
        // --- ALLA ADRESSER MÄTTAS NU FRÅN DIN 64K C++ ARRAY! ---
        // (Om CPU:n vill skriva, tillåter vi det bara i RAM-området < 0xC000)
        if (top.p_ram__we__out.get<bool>() && addr < 49152) {
            oric_ram[addr] = top.p_ram__d__out.get<uint8_t>();
        }
        
        // Injicera datan live (Nu får CPU:n äkta instruktioner från din fil på 0xF5D5!)
        top.p_ram__q__in.set<uint8_t>(oric_ram[addr]);

        std::cout << "[CYKEL " << std::dec << cycle << "] ADDR=" 
                  << std::hex << std::setw(4) << std::setfill('0') << addr << std::endl;

	// Hämta CPU:ns exakta interna Program Counter live från T65-kärnan! [cxxrtl]
uint16_t current_pc = top.cell_p_oric.cell_p_inst__cpu.p_pc.get<uint16_t>();

 printf("PC=%.4x\n", current_pc&0xffff);
 
std::cout << "[CYKEL " << std::dec << cycle 
          << "] PC=" << std::hex << std::setw(4) << std::setfill('0') << current_pc
          << " | RAM_BUS_ADDR=" << std::hex << std::setw(4) << std::setfill('0') << addr 
          << " | WE=" << top.p_ram__we__out.get<bool>() << std::endl;

    }

    return 0;
}
