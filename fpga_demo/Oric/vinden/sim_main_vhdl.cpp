#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <cxxrtl/cxxrtl.h>
#include "oricatmos_sim.h" // Din autogenererade fil från Yosys

// Justera denna faktor N efter hur mycket du överklockar för den asynkrona logiken
const int OVERSAMPLING = 10; 

// Standard PAL/Oric upplösning inkl. overscan/border
const int WIDTH = 400;  
const int HEIGHT = 312; 

const bool chatty_register_log=false;

int main(int argc, char **argv) {
  // 1. Instansiera toppmodulen (Yosys döper klassen efter din VHDL-entitet)
  // Om din yttre vhdl heter 'oric_sim_top' blir det: p_oric__sim__top
  // Heter den fortfarande 'oricatmostop' blir det: p_oricatmostop

  // Istället för std::make_unique, kör vi ren C++11 unique_ptr:
  std::unique_ptr<cxxrtl_design::p_oricatmostop> top(new cxxrtl_design::p_oricatmostop());

    
  std::cout << "Sparkar igång ren kallstart av Oric Atmos..." << std::endl;
  std::cout << "Översamplingsfaktor N = " << OVERSAMPLING << std::endl;

  // Skärmbuffer för att fånga vinjettbilden (R, G, B per pixel)
  std::vector<uint8_t> framebuffer(WIDTH * HEIGHT * 3, 0);

  // 2. Kontrollerad hårdvarureset
  top->p_RESET.set<bool>(true); // Kontrollera om din reset-pinne heter 'reset' eller 'rst'
  for(int i = 0; i < 100; i++) {
    top->p_CLK__24MHz.set<bool>(true);  top->step(); // Kontrollera klockpinnens namn ('clk', 'clk_in'?)
    top->p_CLK__24MHz.set<bool>(false); top->step();
  }
  top->p_RESET.set<bool>(false);
  std::cout << "Reset släppt. Simulerar boot..." << std::endl;

  int x = 0, y = 0;
  bool last_hsync = false;
  bool last_vsync = false;
  unsigned long long total_steps = 0;

  // Vi kör tillräckligt länge för att datorn ska boota och rita ut skärmen
  // 50 miljoner steg räcker oftast för att passera boot-fördröjningen med överklockning
  while (total_steps < 50000000) {

    if (total_steps % 100000 == 0)
      {
	std::cout << "step... = " << total_steps << std::endl;
      }

    if (chatty_register_log || (total_steps % 100000 == 0))
      {
	uint64_t raw_regs = top->p_top__cpu__regs.get<uint64_t>(); // Byt 'p_addr' mot ditt portnamn för adressbussen
	  
	std::printf("Steg: %10llu | Rå CPU-data: 0x%016lX\n", total_steps, raw_regs);
      }
    
    // Klocka HÖG (Rising edge)
    top->p_CLK__24MHz.set<bool>(true);
    top->step();

    // --- VI LÄSER BARA SIGNALER HÄR ---
    // Justera dessa namn så de matchar utgångarna på din yttre VHDL-topp!
    bool hsync = top->p_VIDEO__HSYNC.get<bool>();
    bool vsync = top->p_VIDEO__VSYNC.get<bool>();
    bool r     = top->p_VIDEO__R.get<bool>();
    bool g     = top->p_VIDEO__G.get<bool>();
    bool b     = top->p_VIDEO__B.get<bool>();

    // Synkhantering (Positiv/negativ synk beror på din kärna)
    if (hsync && !last_hsync) {
      x = 0; 
      y++;
    }
    if (vsync && !last_vsync) {
      y = 0; 
    }
        
    last_hsync = hsync;
    last_vsync = vsync;

    // Fånga pixeln (vi tar hänsyn till överklockningen genom att bara rita var N:e cykel per pixel)
    if (total_steps % OVERSAMPLING == 0) {
      if (x < WIDTH && y < HEIGHT) {
	int idx = (y * WIDTH + x) * 3;
	framebuffer[idx]     = r ? 255 : 0;
	framebuffer[idx + 1] = g ? 255 : 0;
	framebuffer[idx + 2] = b ? 255 : 0;
      }
      x++;
    }

    // Klocka LÅG (Falling edge)
    top->p_CLK__24MHz.set<bool>(false);
    top->step();

    total_steps++;
  }

  // 3. Spara ut vinjettbilden till en rå PPM-fil
  std::ofstream ppm_file("oric_vinjett.ppm", std::ios::binary);
  ppm_file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
  ppm_file.write(reinterpret_cast<char*>(framebuffer.data()), framebuffer.size());
  ppm_file.close();

  std::cout << "Simulering färdig! Filen 'oric_vinjett.ppm' har skapats." << std::endl;
  return 0;
}
