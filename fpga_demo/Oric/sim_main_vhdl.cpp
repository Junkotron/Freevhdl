#include <iostream>
#include "oricatmos_sim.h"

// 4 .vcd
#include <cxxrtl/cxxrtl_vcd.h>
#include <fstream>
#include <frame_grab.h>

int time_steps=0;

cxxrtl_design::p_oricatmostop__sim top_design;

// 1. Skapa en behållare för signalerna
cxxrtl::debug_items all_items;

uint32_t* find_address_by_debug_table(std::string key)
{
  // 1. Sök efter adressbussen i samma tabell
  auto it = all_items.table.find(key); // Justera namnet till er signal

  if (it != all_items.table.end()) {
    const cxxrtl::debug_item &addr_item = it->second.front();

    // 2. För en vanlig signal (wire) pekar .curr direkt på råa uint32_t-block
    // Vi behöver inte göra en reinterpret_cast till en hel array, bara läsa första blocket
    uint32_t *raw_data = addr_item.curr;
    return raw_data;
  }
  std::cerr << "OOPS" << std::endl;
  exit(1);
}


void init_ram()
{
  // Spaces not dots
  auto it = all_items.table.find("inst_oricram ram");
  
  if (it != all_items.table.end()) {
    const cxxrtl::debug_item &ram_item = it->second.front(); 
    
    // 1. Storleken finns tryggt i .depth (eftersom debug_item inte är en template)
    size_t memory_size = ram_item.depth; 
    std::cout << "Storlek från debug-objekt: " << memory_size << " ord." << std::endl;
    
    // 2. Gör en reinterpret_cast till en array-pekare av cxxrtl::value<8>
    // Det är detta som är cxxrtl:s interna lagringsformat för ett 8-bitars minne!
    auto *memory_array = reinterpret_cast<cxxrtl::value<8>*>(ram_item.curr);
    
    // 3. Loopa helt typsäkert med vanlig C++ array-indexering
    for (size_t i = 0; i < ram_item.depth; ++i) {
      // Läs värdet typsäkert via .get<uint32_t>()
      uint32_t val = memory_array[i].get<uint32_t>();
      
      // Skriv ett värde (om du vill förfylla minnet):
      // memory_array[i].set<uint32_t>(0x55);
    }
  }
}

void init_ram_old()
{
  // 1. Skapa tabellen över alla signaler (görs oftast i början av din main-funktion)
  
  // 2. Leta upp ditt Oric-RAM (skriv exakt den sökväg du ser i GTKWave, t.ex. "inst oricram mem")

  const char* sstr="inst_oricram ram";
  
  auto it = all_items.table.find(sstr);
  
  if (it == all_items.table.end())
    {
      std::cerr << "Opps cant find anything containing: " << sstr << std::endl;
      exit(1);
    }

  auto &ram_item = it->second.front();

  cxxrtl::value<8> *memory_data = (cxxrtl::value<8>*)ram_item.curr;

    
  // 3. Kör loopen! Det här fungerar till 100 % utan att bry sig om -noflatten-typer
  for (size_t i = 0; i < ram_item.depth/2; ++i) {
    // Här kan du läsa eller skriva direkt på index i:
    if (i % 2 == 0) {
      memory_data[i].set<uint32_t>(0x55);
    }
    else {
      memory_data[i].set<uint32_t>(0xAA);
    }
  }
  
}


void set_master_clock(bool val)
{
  // Drive top-level inputs (Verify underscore quantity in gen_design.h)
  top_design.p_CLK__24MHz.set<uint8_t>(val);

  // For the sceptic
  // std::cout << "Simulation Input:  " << (int)top_design.p_CLK__24MHz.get<uint8_t>() << std::endl;
}

void set_reset(bool val)
{
  top_design.p_RESET.set<uint8_t>(val);  
}

static bool do_vcd=false;
static bool do_frames=false;



void dump_vcd(cxxrtl::vcd_writer& vcd, std::ofstream& vcd_file)
{
  if (do_vcd)
    {
      // Dumpa tillstånd efter en halv klockcykel
      vcd.sample(time_steps++);
      vcd_file << vcd.buffer; // 2. Skriv ut den till filen
      vcd.buffer.clear();     // 3. Töm inför nästa varv
    }
}

static int stop_sim_step=5000;
static int print_intervals=100;

void usage(const char* pname)
{
  std::cerr << "Usage: " << pname << " vcd/frames [sim steps] [print interval]" << std::endl;
  std::cerr << "zero means loop forever and no print respectively" << std::endl;
  exit(1);
}


void options(int argc, char *argv[])
{
  if (argc<2)
    {
      usage(argv[0]);
    }

  std::string arg1 = argv[1];

  if (arg1 == "vcd") do_vcd=true;
  else if (arg1 == "frames") do_frames=true;
  else usage(argv[0]);

  if (argc>=3)
    {
      stop_sim_step=atoi(argv[2]);
    }
  if (argc==4)
    {
      print_intervals=atoi(argv[3]);
    }

  printf("stop_sim_step=%d print_intervals=%d\n", stop_sim_step, print_intervals);
}

int main(int argc, char *argv[]) {

  options(argc, argv);
  
  VideoCapture vcap;
  
  // Öppna en filström för att spara vcd-filen
  std::ofstream vcd_file("waveforms.vcd");

  // 24 MHz 
  if (do_vcd) vcd_file << "$timescale 21ns $end\n"; 
  
  // Skapa VCD-skrivaren och spara designens signalhierarki
  cxxrtl::vcd_writer vcd;
  //  top_design.debug_info(vcd.debug_item());


  cxxrtl::debug_scopes scopes;

  // 3. Anropa debug_info med pekare till objekten samt en tom rotsökväg ""
  top_design.debug_info(&all_items, &scopes, "");

  // 3. Registrera dessa signaler i din VCD-skrivare
  if (do_vcd) vcd.add(all_items);

  init_ram();
  
  std::cout << "Sim done init..." << std::endl;

  set_reset(true);
  top_design.step();

  float tstep = 1.0/24000000.0;

  // Find a ref to the cpu addr
  // Spaces not dots...
  uint32_t* cpu_addr = find_address_by_debug_table("oric cpu_ad");

  uint32_t* cpu_di = find_address_by_debug_table("oric cpu_di");

  // todo have this in real time calculated instead 1000 = about 40 us
  for (int i=0;;i++)
    {      
      if (stop_sim_step)
	if (i>=stop_sim_step)  break;

      if (i==20) set_reset(false);
      
      set_master_clock(true);
    
      // Evaluate the simulation. This calls your custom C++ blackbox eval() automatically!
      top_design.step();
      top_design.debug_eval();
      
      // This is the positiv edge of main clock
      if (do_frames)
	{
	  auto& fula = top_design.cell_p_oric.cell_p_inst__ula;
	  
	  vcap.process_step(!fula.p_hsync.get<bool>(),
			    !fula.p_vsync.get<bool>(),
			    fula.p_r.get<bool>(),
			    fula.p_g.get<bool>(),
			    fula.p_clk__pix.get<bool>()
			    );
	}
      
      if (do_vcd) {
	dump_vcd(vcd, vcd_file);
      }

      set_master_clock(false);
    
      // Evaluate the simulation. This calls your custom C++ blackbox eval() automatically!
      top_design.step();

      if (do_vcd)
	{
	  dump_vcd(vcd, vcd_file);
	}

      if (print_intervals)
	if (i%print_intervals == 0)
	  {
	    printf("i=%d, t=%f\n", i, i*tstep);
	    printf("addr=%.4X: %.2X\n", (*cpu_addr) & 0xffff, *cpu_di);
	  }
      
    }

  if (do_vcd)
    {
      vcd_file.flush();
      vcd_file.close();
    }
  
  return 0;
}
