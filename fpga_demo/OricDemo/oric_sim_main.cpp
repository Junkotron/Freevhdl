#include <iostream>
#include "cpu_ula_sim.h"
#include "snapshot_utils.h"

// 4 .vcd
#include <cxxrtl/cxxrtl_vcd.h>
#include <iostream>
#include <fstream>
#include "frame_grab.h"

// Save & load vid snapshots...


using namespace std;





// TODO this more or less heuristic beast was used to locate the rom name in the simulator
// yosys mangling is truly insane so this util might be useful for just searching especially
// massive vectors in its own main with a a command line arg name and size 
void find_signal_by_size(cxxrtl::debug_items& items)
{
  const cxxrtl::debug_item *target_rom = nullptr;
  string found_name = "";

  for (const auto& pair : items.table) {
    // Vi letar efter Atmos ROM (inst_rom1) som har djupet 16384
    if (pair.first.find("inst_rom0") != string::npos) {
      const cxxrtl::debug_item &item = pair.second.front();
      if (item.depth == 16384) {
	target_rom = &item;
	found_name = pair.first;

	cerr << "name 16k <<" << found_name << ">>" << endl;

	//	break; // Hittad! Avbryt sökningen.
      }
    }
  }

  // TODO Some flag here and a message if we did not find anything
}

void patch_rom(cxxrtl::debug_items& items)
{
  
  // Spaces not dots, this one was really hard to find, check todo above
  auto it = items.table.find("oric inst_rom0 :1518");
  

  if (it != items.table.end()) {
    const cxxrtl::debug_item &rom_item = it->second.front(); 
    
    // 1. Storleken finns tryggt i .depth (eftersom debug_item inte är en template)
    size_t memory_size = rom_item.depth; 
    cout << "(ROM) Storlek från debug-objekt: " << memory_size << " ord." << endl;
    
    // 2. Gör en reinterpret_cast till en array-pekare av cxxrtl::value<8>
    // Det är detta som är cxxrtl:s interna lagringsformat för ett 8-bitars minne!

    auto *memory_array = reinterpret_cast<cxxrtl::value<8>*>(rom_item.curr);

    // Patcha rom:    

    // adress in i själva minnet, dra bort offsetten
    uint32_t addr = 0xFA3C-0xC000;
    
    memory_array[addr].set<uint32_t>(0x4C); // JMP opcode hoppar direkt till "det gick bra"
    memory_array[addr+1].set<uint32_t>(0x44); // JMP opcode hoppar direkt till "det gick bra"
    memory_array[addr+2].set<uint32_t>(0xFA); // JMP opcode hoppar direkt till "det gick bra"
  }
  else {
    cerr << "OOOPS no rom on this name\n";
    exit(1);
  }
}



void init_ram(cxxrtl::debug_items& items)
{
  // Spaces not dots
  auto it = items.table.find("inst_oricram ram");
  
  if (it != items.table.end()) {
    const cxxrtl::debug_item &ram_item = it->second.front(); 
    
    // 1. Storleken finns tryggt i .depth (eftersom debug_item inte är en template)
    size_t memory_size = ram_item.depth; 
    cout << "Storlek från debug-objekt: " << memory_size << " ord." << endl;
    
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
  else
    {
      cerr << "OOOPS no ram on this name\n";
      exit(1);
    }
}

void set_master_clock(cxxrtl_design::p_oricatmostop& top, bool val)
{
  // Drive top-level inputs (Verify underscore quantity in gen_design.h)
  top.p_CLK__24MHz.set<uint8_t>(val);

  // For the sceptic
  // cout << "Simulation Input:  " << (int)top.p_CLK__24MHz.get<uint8_t>() << endl;
}

void set_reset(cxxrtl_design::p_oricatmostop& top, bool val)
{
  top.p_RESET.set<uint8_t>(val);  
}

static bool do_vcd=false;
static bool do_frames=false;

// TODO what does this parameter actually mean or do?
static int time_steps=0;
void dump_vcd(cxxrtl::vcd_writer& vcd, ofstream& vcd_file)
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
  cerr << "Usage: " << pname << " run/vcd/frames [sim steps] [print interval]" << endl;
  cerr << "zero means loop forever and no print respectively" << endl;
  exit(0);
}


void options(int argc, char *argv[])
{
  if (argc<2)
    {
      usage(argv[0]);
    }

  string arg1 = argv[1];

  if (arg1 == "vcd") do_vcd=true;
  else if (arg1 == "frames") do_frames=true;
  else if (arg1 != "run") usage(argv[0]);

  if (argc>=3)
    {
      stop_sim_step=atoi(argv[2]);
    }
  if (argc==4)
    {
      print_intervals=atoi(argv[3]);
    }

  // TODO cout allover
  printf("stop_sim_step=%d print_intervals=%d\n", stop_sim_step, print_intervals);
}

void vcd_init(cxxrtl::vcd_writer& vcd, ofstream& vcd_file)
{
  if (do_vcd)
    {
      vcd_file.open("waveforms.vcd");

      // 24 MHz : TODO parameter?
      if (do_vcd) vcd_file << "$timescale 21ns $end\n";
    }
}

void vcd_add_items(cxxrtl::vcd_writer& vcd, cxxrtl::debug_items& items)
{
  vcd.add(items);  
}


static string as_bit(uint32_t* num)
{
  return (*num) ? "1" : "0";
}


static string as_hex2(uint32_t* num)
{
  char tmp[100];
  sprintf(tmp, "%.2X", (*num)&0xff);
  return tmp;
}

static string as_hex4(uint32_t* num)
{
  char tmp[100];
  sprintf(tmp, "%.4X", (*num)&0xffff);
  return tmp;
}

int main(int argc, char *argv[]) {

  options(argc, argv);

  // Used if we capture frames
  VideoCapture vcap;
  
  cxxrtl_design::p_oricatmostop top;
  cxxrtl::debug_items all_items;
  cxxrtl::debug_scopes scopes;

  ofstream vcd_file;
  cxxrtl::vcd_writer vcd;

  if (do_vcd) vcd_init(vcd, vcd_file);
  
  // Exempel på din externa 64KB minnesmatris för Oric-RAM
  uint8_t memory_array[65536];

  // 3. Anropa debug_info med pekare till objekten samt en tom rotsökväg ""
  top.debug_info(&all_items, &scopes, "");

  if (do_vcd) vcd_add_items(vcd, all_items);
  
  // Nollställ det simulerade RAM-minnet todo: pattern init?
  std::memset(memory_array, 0, sizeof(memory_array));

  std::cout << "--- STARTAR ORIC SIMULATOR ---" << std::endl;

  // Försök ladda snapshot från disk
  bool restarted = load_system_snapshot(top, memory_array, sizeof(memory_array));
    
  size_t start_cycle = 0;
  size_t end_cycle = stop_sim_step;

  if (!restarted) {
    std::cout << "[BOOT] Ingen snapshot hittades. Kör normal uppstart..." << std::endl;

    set_reset(top, true);
    top.step(); // Grundinitiering av CXXRTL-grafen
    
  } else {
    std::cout << "[BOOT] Återstartad från snapshot! Fortsätter simulering..." << std::endl;

    // TODO spara faktiskt klockcykel
    start_cycle = end_cycle; // Om vi laddade, låtsas vi att vi redan kört 10 cykler
  }


  // Find references to things we want to track
  uint32_t* cpu_addr = find_address_by_debug_table(all_items, "oric cpu_ad");
  assert (cpu_addr != nullptr);
  uint32_t* cpu_di = find_address_by_debug_table(all_items, "oric cpu_di");
  assert (cpu_di  != nullptr);

  // TODO: did these move?
  //  uint32_t* irq = find_address_by_debug_table(all_items, "oric inst_via o_irq_l");
  //  assert (irq != nullptr);
  //  uint32_t* r_w_n = find_address_by_debug_table(all_items, "oric inst_cpu r_w_n");  
  //  assert (r_w_n != nullptr);

  uint32_t* cpu_do = find_address_by_debug_table(all_items, "oric cpu_do");
  assert (cpu_do != nullptr);

  // Simuleringsloop todo: det vore trevlig att spara cycle värdet också vid
  // snapshot så man kan snurra vidare på samma cycle
  //  for (size_t cycle = start_cycle; cycle < end_cycle; ++cycle) {
  for (size_t cycle = start_cycle; ; ++cycle) {
    
    if (!restarted && cycle == 20)
      {
	set_reset(top, false);
      }

    // TODO: we need to make less printouts or frame dump will take ludicrous time
    std::cout << "[CYKEL " << cycle << "] Tickar hårdvaruklockan..." << std::endl;

    cout << "ADDR=" << as_hex4(cpu_addr) << ":" << as_hex2(cpu_di) << as_hex2(cpu_do) << endl;
    
    // Tick tack...
    set_master_clock(top, false);
    top.step();
    set_master_clock(top, true);
    top.step();

    // This is the positiv edge of main clock
    if (do_frames)
      {
	// TODO: this frame dumping nightmare just has to wait
	auto& fula = top.cell_p_oric.cell_p_inst__ula;
	
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
    
    // Demotrigg: Spara snapshot vid cykel end_cycle om detta är en ren nyboot
    if (!restarted && cycle == end_cycle) {
      std::cout << "[TRIGGER] Cykel " << end_cycle << " nådd! Sparar och avbryter för test..." << std::endl;
      save_system_snapshot(top, memory_array, sizeof(memory_array));
      std::cout << "Kör programmet en gång till för att verifiera automatisk load!" << std::endl;
      return 0; 
    }
  }

  std::cout << "--- SIMULERING SLUTFÖRD UTAN KRASCH ---" << std::endl;
  return 0;
}


