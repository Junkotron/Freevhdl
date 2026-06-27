#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include "cpu_ula_sim.h" // Din fungerande hårdvaruheader
#include "snapshot_utils.h" // För att ladda och spara systemtillstånd
#include "retro_monitor.h"
#include "keyboard_io.h"

using namespace std;

// Vi uppgraderar minnet till fulla 64K så att det täcker ALLA adresser (0x0000 - 0xFFFF)
uint8_t oric_ram[65536];



// Måltillstånd för din CPU (Används i kalloop för att veta var vi ska landa)
struct CPUState {
    uint8_t a  = 0x42;       
    uint8_t x  = 0x11;       
    uint8_t y  = 0x22;       
    uint8_t s  = 0xFD;       
    uint8_t p  = 0x24;       
    uint16_t pc = 0xF5CD;    // Den legendariska magiska BASIC Cold Start-adressen!
};

// Din skugg-ROM-generator (Används enbart i kalloop)
// This is only used in cold boot we set some stuff to enable saving of snapshot
// the correct way
void setup_shadow_rom(CPUState state) {
    uint16_t inject_addr = 0x0000;
    oric_ram[0xFFFC] = inject_addr & 0xFF;
    oric_ram[0xFFFD] = (inject_addr >> 8) & 0xFF;
    
    auto write_code = [&](uint8_t byte) { oric_ram[inject_addr++] = byte; };
    
    write_code(0xA9); write_code(state.a); // LDA #val
    write_code(0xA2); write_code(state.x); // LDX #val
    write_code(0xA0); write_code(state.y); // LDY #val
    write_code(0xA9); write_code(state.p); // LDA #p_val
    write_code(0x48);                      // PHA
    write_code(0x28);                      // PLP
    write_code(0xA2); write_code(state.s); // LDX #s_val
    write_code(0x9A);                      // TXS
    write_code(0x4C);                      // JMP abs
    write_code(state.pc & 0xFF);           
    write_code((state.pc >> 8) & 0xFF);    
}

// This is the actual diag program we are trying to run regardless of
// cold or warm boot
void patch_diag_rom(cxxrtl::debug_items& items) {
    const cxxrtl::debug_item *rom_item = nullptr;
    string found_name = "";

    // 1. DYNAMISK SÖKNING: Loopa igenom tabellen precis som i find_signal_by_size
    for (const auto& pair : items.table) {
        // Letar efter ett namn som innehåller "inst_rom0" eller "rom" 
        // och som har det exakta djupet 16384 (16KB)
        if (pair.first.find("inst_rom0") != string::npos || pair.first.find("rom0") != string::npos) {
            const cxxrtl::debug_item &item = pair.second.front();
            if (item.depth == 16384) {
                rom_item = &item;
                found_name = pair.first;
                break; // Hittad! Avbryt sökningen.
            }
        }
    }

    // 2. EXEKVERA PATCHEN OM DEN HITTADES
    if (rom_item != nullptr) {
        cout << "🛠️  [ROM PATCH] Hittade ROM under namnet: <<" << found_name << ">> (" << rom_item->depth << " ord)" << endl;
        
        auto *memory_array_rom = reinterpret_cast<cxxrtl::value<8>*>(rom_item->curr);

        // Vår stenhårda Jönssonligan-soppa (testprogrammet) flyttat till ROM-säker mark!
        // Korrigerad, stenhård 6502-soppa för boot-adress $F88F
        uint8_t test_program[] = {
            0xA9, 0x42,       // $F88F: LDA #$42  (Initialvärde)
            0x8D, 0x00, 0xBB, // $F891: STA $BB00
            0xA9, 0xAA,       // $F894: A: LDA #$AA -> Start för Loop A
            0x8D, 0x00, 0xBB, // $F896: STA $BB00
            0xAD, 0x00, 0x03, // $F899: LDA $0300 (Läs knapp S1)
            0xC9, 0x01,       // $F89C: CMP #$01
            0xF0, 0xF4,       // $F89E: BEQ A     -> Hoppar 12 bytes bakåt till $F894
            0xA9, 0xBB,       // $F8A0: B: LDA #$BB -> Start för Loop B
            0x8D, 0x00, 0xBB, // $F8A2: STA $BB00
            0x4C, 0xA0, 0xF8  // $F8A5: JMP B     -> Hoppa absolut till $F8A0 (Ändrad till A0!)
        };

        // Vi startar exakt på $F88F och drar av Orics ROM-offset ($C000)
        uint32_t addr = 0xF88F - 0xC000;
        
        // Tryck in hela soppan direkt i kisel-ROM-grafen!
        for (size_t i = 0; i < sizeof(test_program); ++i) {
            memory_array_rom[addr + i].set<uint32_t>(test_program[i]);
        }
        cout << "✨ [ROM PATCH] Success! Testprogram injicerat direkt på boot-adressen $F88F." << endl;
    }
    else {
        cerr << "OOOPS! Hittade inget 16KB ROM-minne med 'inst_rom0' i tabellen.\n";
        exit(1);

    }
    cout << "🛠️  [ROM PATCH] Success! Testprogram injicerat direkt på boot-adressen $F88F." << endl;
}

void spy_on_port_c(const uint8_t* ram_buffer) {
    static uint8_t last_c_val = 0xFF;
    uint8_t current_c_val = ram_buffer[0xBB00];
    
    // Skriv BARA ut när minnet faktiskt muterar, så slipper vi terminal-lag!
    if (current_c_val != last_c_val) {
        cout << "\n[📺 MONITOR] Port C ($BB00) ändrades till: $" 
                  << hex << setw(2) << setfill('0') << (int)current_c_val << "\n";
        last_c_val = current_c_val;
    }
}

void spy_on_port_c_kisel(uint32_t* ram_ad, uint32_t* ram_d, uint32_t* ram_we) {
    if (!ram_ad || !ram_d || !ram_we) return;
    
    // Vi reagerar BARA när hårdvaran faktiskt gör en aktiv SKRIVNING till $BB00!
    if (*ram_we && (*ram_ad & 0xFFFF) == 0xBB00) {
        static uint8_t last_c_val = 0xFF;
        uint8_t current_c_val = (uint8_t)(*ram_d & 0xFF);
        
        if (current_c_val != last_c_val) {
            std::cout << "\n[📺 MONITOR - KALL] Processorn skrev direkt till Port C ($BB00): $" 
                      << std::hex << std::setw(2) << std::setfill('0') << (int)current_c_val << "\n";
            last_c_val = current_c_val;
        }
    }
}


// Vårt globala eller externa terminalobjekt
Keyboard g_keyboard; 

// En ren, datatät struktur för knapparnas tillstånd
struct HardwareButtons {
    uint8_t s1;       // 01h = Uppe, 00h = Nere
    bool s2_snapshot; // True om S2 är nedtryckt
};

struct KeyStatus {
    uint8_t s1;       // Levererar 01h (uppe) eller 00h (nere)
    bool s2_snapshot; // True om vi ska frysa och dumpa
};

// Vår fristående kontrollfunktion - helt fri från timers!
 KeyStatus check_keys() {
    KeyStatus status = {0x01, false}; // Standard: S1 uppe (01h), ingen S2

    while (true) {

        // Vi använder din egna, säkra klassmetod!
        // Den returnerar -1 om bufferten är tom tack vare fcntl i init()
        int ch = g_keyboard.get_key(); 
	
        if (ch == -1) { 
            break; // Bufferten är tom, hoppa ut på noll nanosekunder!
        }
        
        if (ch == '1') {
            status.s1 = 0x00; // S1 detekterad som NERE
        } 
        else if (ch == '2') {
            status.s2_snapshot = true; // S2 triggad!
        }
    }
    
    return status;
}


// =============================================================================
// DIAGNOSTIK OCH I/O-TESTFÄLLOR (UTBRUTNA FRÅN VARMLOOP)
// =============================================================================


bool io_test(uint16_t addr, cxxrtl_design::p_oricatmostop& top_warm) {
    // Synkron buss-avlyssning av VIA-fällan $0300 helt utan timers!
    if (addr == 0x0300) {
        KeyStatus keys = check_keys();
        
        // S2-knappen: Avbryt omedelbart och dumpa kisel-strukturen
        if (keys.s2_snapshot) {
            cout << "\n[S2] Snapshot triggad manuellt! Sparar tillstånd till disk...\n";
            save_system_snapshot(top_warm, oric_ram, sizeof(oric_ram));
            return true; // Returnera true till loopen för att signalera nödstopp
        }
        
        // S1-knappen: Leverera status direkt på databussen (01h = uppe, 00h = nere)
        top_warm.p_ram__q__in.set<uint8_t>(keys.s1);
    }
    return false; // Ingen I/O-avbrutning, dundra vidare i loopen
}


// =============================================================================
// SPÅR 1: DET RENA KALLSTARTS-RIKET (Skugg-ROM & Retro Monitor)
// =============================================================================
void kallloop() {
    cout << "❄️ [KALLSTART] Kör skugg-ROM och gipsvagga...\n";
    
    cxxrtl_design::p_oricatmostop top_cold;
    CPUState target_cpu;

    
    // Generera skuggkoden i botten av minnet
    setup_shadow_rom(target_cpu);
    
    // Hämta tabellen lokalt och låt Retro Monitor bita sig fast autonomt
    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top_cold.debug_info(&items, &scopes, "");
    RetroMonitor::init(items, target_cpu.pc, 64);

    // Lägg till dessa där du hittar cpu_addr, cpu_di osv. i main():
    uint32_t* kisel_ram_ad = find_address_by_debug_table(items, "oric ram_ad");
    uint32_t* kisel_ram_d  = find_address_by_debug_table(items, "oric ram_d");
    uint32_t* kisel_ram_we = find_address_by_debug_table(items, "oric ram_we");
    
    assert(kisel_ram_ad != nullptr);
    assert(kisel_ram_d  != nullptr);
    assert(kisel_ram_we != nullptr);

    patch_diag_rom(items);
    
    // Vi kollar även adressen lokalt för vår progress-tracker
    auto lookup = [&](const string& key) -> uint32_t* {
        auto it = items.table.find(key);
        if (it != items.table.end() && !it->second.empty()) return it->second.front().curr;
        return nullptr;
    };
    uint32_t* ptr_addr = lookup("oric inst_ula addr");
    
    // Minimum hårdvaru-reset för kallstarten via dina skarpa portar
    top_cold.p_RESET.set<bool>(true);
    top_cold.p_CLK__24MHz.set<bool>(false); top_cold.step();
    top_cold.p_CLK__24MHz.set<bool>(true);  top_cold.step();
    top_cold.p_RESET.set<bool>(false);
    
    cout << "🚀 Startar skugg-ROM-loopen. Letar efter $" << hex << target_cpu.pc << "...\n";

    std::cout << "🚀 Startar skugg-ROM-loopen...\n";
    
    // Den stenhårda, fungerande 500 000-cyklersloopen
    for (uint64_t cycle = 0; cycle < 500000; ++cycle) {
        
        // 1. KLOCKAN GÅR HÖG (Körs överst som du vill ha det)
        top_cold.p_CLK__24MHz.set<bool>(true);
        top_cold.step();

        // 2. TIMING-FRI KNAPPKOLL (Slukar skräp utan Enter-lås)
        KeyStatus keys = check_keys();
        if (keys.s2_snapshot) {
            std::cout << "\n[S2 DETEKTERAD I KALLLOOP] Manuellt nödstopp! Avbryter...\n";
            break; 
        }

        // 3. GEMENSAM MONITOR-RAPPORTERING (Läser råa kisel-transienter)
        spy_on_port_c_kisel(kisel_ram_ad, kisel_ram_d, kisel_ram_we);

        // Progress-räknare: Livstecken under de första cyklerna
        if (cycle < 2000 && ptr_addr) {
            std::cout << "  ⏱️  Kallstart Cykel " << std::dec << cycle 
                      << " | CPU-buss just nu på adress: $" << std::hex << (*ptr_addr & 0xFFFF) << "\n";
        }

        if (cycle % 100000 == 0 && ptr_addr) {
            std::cout << "  ⏱️  Kallstart Cykel " << std::dec << cycle 
                      << " | CPU-buss just nu på adress: $" << std::hex << (*ptr_addr & 0xFFFF) << "\n";
        }
        
        // Busshantering via Retro Monitors tick
        if (RetroMonitor::tick(cycle, oric_ram)) {
            std::cout << "✨ [BRYTPUNKT DETEKTERAD] CPU landade på $" << std::hex << target_cpu.pc << "!\n";
            load_real_atmos_rom(oric_ram, "Oric_MiSTer_src/roms/basic11b.rom");
            break; 
        }
        
        // 4. KLOCKAN GÅR LÅG (Nederst i loopen)
        top_cold.p_CLK__24MHz.set<bool>(false);
        top_cold.step();
    }

    
    /*
    // Ökat till 500 000 cykler så att 24MHz-klockan hinner ge CPU:n tillräckligt med steg!
    for (uint64_t cycle = 0; cycle < 500000; ++cycle) {
        top_cold.p_CLK__24MHz.set<bool>(true);
        top_cold.step();
        
        KeyStatus keys = check_keys();
        if (keys.s2_snapshot) {
            cout << "\n[S2 DETEKTERAD I KALLLOOP] Manuellt nödstopp! Avbryter kallstart direkt...\n";
            break; // Hoppar direkt ut ur loopen och ner till save_system_snapshot under!
        }

	// Progress-räknare: Livstecken så du ser att kallstarten lever och var den är i minnet
        if (cycle < 2000 && ptr_addr) {
            cout << "  ⏱️  Kallstart Cykel " << dec << cycle 
                      << " | CPU-buss just nu på adress: $" << hex << (*ptr_addr & 0xFFFF) << "\n";
        }

        // Progress-räknare: Livstecken så du ser att kallstarten lever och var den är i minnet
        if (cycle % 100000 == 0 && ptr_addr) {
            cout << "  ⏱️  Kallstart Cykel " << dec << cycle 
                      << " | CPU-buss just nu på adress: $" << hex << (*ptr_addr & 0xFFFF) << "\n";
        }

	spy_on_port_c(oric_ram);
	
        // Busshantering för kallstarten via Retro Monitors tick
        if (RetroMonitor::tick(cycle, oric_ram)) {
            cout << "✨ [BRYTPUNKT DETEKTERAD] CPU landade på $" << hex << target_cpu.pc << "!\n";
            // Triggad! Skriv över med det riktiga Atmos ROM:et direkt i utils
            load_real_atmos_rom(oric_ram, "Oric_MiSTer_src/roms/basic11b.rom");
            break; // Bryt så du hinner se din vackra dump på skärmen!
        }
        
        top_cold.p_CLK__24MHz.set<bool>(false);
        top_cold.step();
    }
    */
    
    // Oki då har vi kört våra steg, dags att spara tillståndet stabilt!
    save_system_snapshot(top_cold, oric_ram, sizeof(oric_ram));
}


// =============================================================================
// SPÅR 2: DET SKARPA VARMSTARTS-RIKET (Snapshot & Maxhastighet)
// =============================================================================


void varmloop() {
    cout << "\n==================================================================\n";
    cout << " 🔥 [VARMSTART] FULL HASTIGHET ENLIGT ARKIV-MODELL \n";
    cout << "==================================================================\n";
    
    cxxrtl_design::p_oricatmostop top_warm;
    
    // 1. Läs in mjukvaruminnet (oric_ram arrayen) från filen först via utils
    load_system_snapshot(top_warm, oric_ram, sizeof(oric_ram), false);

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top_warm.debug_info(&items, &scopes, "");
    
    // Kör in det "riktiga" diagnosprogrammet (till skillnade från fusk progget
    // som lurar simulatorn att starta på rätt adress)
    patch_diag_rom(items);

    // =========================================================================
    // ⚔️ HARDCORE BOOT-RITUAL (HÄMTAT DIREKT FÖR HAND FRÅN ARKIVET)
    // =========================================================================
    top_warm.p_RESET.set<bool>(true);
    for (int i = 0; i < 32; i++) {
        top_warm.p_CLK__24MHz.set<bool>(false); top_warm.step();
        top_warm.p_CLK__24MHz.set<bool>(true);  top_warm.step();
    }
    
    top_warm.p_RESET.set<bool>(false);
    top_warm.p_CLK__24MHz.set<bool>(false); top_warm.step();
    top_warm.p_CLK__24MHz.set<bool>(true);  top_warm.step();

    auto& cpu = top_warm.cell_p_oric.cell_p_inst__cpu;
    cpu.p_pc.set<uint16_t>(0xF5D5);     
    cpu.p_abc.set<uint16_t>(0x00);      
    cpu.p_p.set<uint16_t>(0x24);        
    cpu.p_s.set<uint16_t>(0xFD);        

    cpu.p_rstcycle.set<bool>(false); 
    cpu.p_irqcycle.set<bool>(false); 
    cpu.p_nmicycle.set<bool>(false); 
    cpu.p_mcycle.set<uint16_t>(0);   

    top_warm.step();
    // =========================================================================

    cout << "🚀 Släpper lös 20 miljoner cykler i maxhastighet...\n";
    
    // Kör 20 miljoner cykler
    for (uint64_t cycle = 0; cycle < 20000000; ++cycle) {
        
        top_warm.p_CLK__24MHz.set<bool>(false); 
        top_warm.step();

        top_warm.p_CLK__24MHz.set<bool>(true);  
        top_warm.step(); 

        // --- DE PUBLIKA ARKIV-PORTARNA ---
        uint16_t addr = top_warm.p_ram__ad__out.get<uint16_t>();
        bool is_we    = top_warm.p_ram__we__out.get<bool>();

        // 1. ANROPA I/O-TESTFÄLLAN (Håller koll på S1 och S2)
        if (io_test(addr, top_warm)) {
            break; // S2 tryckt! Bryt varmloopen och stäng ner simulatorn
        }
	
        if (is_we) { 
            if (addr < 49152) { 
                oric_ram[addr] = top_warm.p_ram__d__out.get<uint8_t>();
            }
        } else { 
            top_warm.p_ram__q__in.set<uint8_t>(oric_ram[addr]);
        }

	spy_on_port_c(oric_ram);
	
        // 💾 TRACING-SKYDD: Dumpa BARA de absolut första 20 cyklerna för att se starten!
        // Detta förhindrar att prompten hackar eller laggar ihop.
        if (cycle < 2000) {
            uint16_t current_pc = cpu.p_pc.get<uint16_t>();
            cout << "[STARTCYKEL " << dec << cycle 
                      << "] PC=" << hex << setw(4) << setfill('0') << current_pc
                      << " | BUSS=$" << hex << setw(4) << setfill('0') << addr << "\n";
        }
        
        // Progress-räknare: Skriv bara en rad var 5:e miljon cykel så maskinen kan flyga!
        if (cycle >= 20 && cycle % 5000000 == 0) {
            uint16_t current_pc = cpu.p_pc.get<uint16_t>();
            cout << "  ⏱️  Puls Cykel " << dec << cycle 
                      << " | PC=$" << hex << current_pc << " | Buss=$" << addr << "\n";
        }
    }

    cout << "\n🏁 Varmstartsloopen färdigkörd!\n";
}


// =============================================================================
// 3. HUVUDMENYN (VAL GÖR MAN TIDIGT I LIVET)
// =============================================================================
int main() {
    // Sökvägar till dina stenhårda filkrav
    const string ram_path = "oric_ram.bin";
    const string reg_path = "oric_registers.snapshot";

    g_keyboard.init();
    
    cout << "==================================================================\n";
    cout << " ⚔️  ORIC ATMOS INJECTION ENGINE v2.0 - BOOT SELECTOR\n";
    cout << "==================================================================\n";

    // Vägvalet sker omedelbart helt i mjukvara via din anala grindvakt!
    if (verify_snapshot_presence(ram_path, reg_path)) {
        varmloop(); // Framåt marsch!
    } else {
        kallloop(); // Trygg reträtt till sandlådan
    }

    cout << "\n🏁 [MAIN] Labbsessionen avslutad.\n";

    return 0;
}
