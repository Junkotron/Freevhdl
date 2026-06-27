#include <iostream>
#include <iomanip>
#include "obj_dir/my6502_core_sim.h" // Vår synkade header

int main() {
    std::cout << "==================================================================\n";
    std::cout << " ⚔️  SIMPLE_6502 -  FULL LOGIC ANALYZER COCKPIT ACTIVE\n";
    std::cout << "==================================================================\n";

    cxxrtl_design::p_top__t65__system top; 
    top.eval(); 

    std::cout << "▶️  Kisel-propellern snurrar i fullgas!\n";
    std::cout << "▶️  Övervakar CPU-kärnans råa portar synkront...\n\n";

    // Vi kör 65 cykler så vi hinner se startsekvensen slå över i din register-loop
    uint64_t cycle = 0;
    while (cycle < 6000) {
        
        // --- 1. KLOCKAN HÖG (Den aktiva kisel-flanken!) ---
        top.p_clk__in.set<bool>(true);
        top.step();
        top.eval(); 

        // 🚨 UTREADNING AV DINA NYA, ULTRA-SKARPA PORTAR!
	bool     clk_diag = top.p_CLK__1MHz.get<bool>(); 
        uint8_t  hi       = top.p_addrhi.get<uint8_t>();
        uint8_t  lo       = top.p_addrlo.get<uint8_t>();
        bool     rw_n     = top.p_cpu__rw__n.get<bool>();
        uint32_t cpu_addr = top.p_cpu__addr.get<uint32_t>(); // 24-bitars bussen!
        uint8_t  din      = top.p_cpu__data__in.get<uint8_t>();
        uint8_t  dout     = top.p_cpu__data__out.get<uint8_t>();
        uint8_t  res_n_out  = top.p_reset__n.get<uint8_t>();

        // Skriv ut den fullständiga kisel-statusen
        std::cout << "[CYKEL " << std::dec << std::setw(2) << std::setfill('0') << cycle << "] "
                  << "CLK_1MHz=" << (clk_diag ? "HÖG (1)" : "LÅG (0)")
                  << "BUS=$" << std::hex << std::setw(2) << std::setfill('0') << (int)hi 
                               << std::setw(2) << std::setfill('0') << (int)lo
                  << " | RES_N=" << (res_n_out ? "RUNNING " : "RESETTING")
                  << " | CPU_ADR=$" << std::setw(6) << std::setfill('0') << cpu_addr
                  << " | R/W=" << (rw_n ? "READ " : "WRITE")
                  << " | DIN=$" << std::setw(2) << std::setfill('0') << (int)din
                  << " | DOUT=$" << std::setw(2) << std::setfill('0') << (int)dout << "\n";

        // Vårt bevisade LED-trycks-fångst mot extern adressavlyssning ($4000)
        if (hi == 0x40 && lo == 0x00 && !rw_n) {
            std::cout << "  ✨ [LED OUTPUT TRIGGER] Dioderna tänds! CPU skickade ut: $" 
                      << std::hex << (int)dout << " via DOUT-porten !!!\n";
        }

        // --- 2. KLOCKAN LÅG (Passiv flank) ---
        top.p_clk__in.set<bool>(false);
        top.step();
        top.eval(); 

        cycle++;
    }

    std::cout << "\n🏁 Analys klar! Varenda bit fångad i realtid.\n";
    return 0;
}
