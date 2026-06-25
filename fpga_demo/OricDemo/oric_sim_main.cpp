#include <iostream>
#include "cpu_ula_sim.h"

// 4 .vcd
#include <cxxrtl/cxxrtl_vcd.h>
#include <fstream>
#include <frame_grab.h>

// Save & load vid snapshots...

#include <iostream>
#include <fstream>
#include <cxxrtl/cxxrtl.h>


#include <iostream>
#include <fstream>
#include <cxxrtl/cxxrtl.h>

using namespace std;


// ============================================================================
// 1. SKOTTSÄKER NAMNBASERAD SPARFUNKTION (Anpassad för din debug_info)
// ============================================================================
void save_system_snapshot(cxxrtl_design::p_oricatmostop &top, void* my_ram, size_t ram_size) {
    std::ofstream ram_os("oric_ram.bin", std::ios::binary);
    if (ram_os.is_open()) {
        ram_os.write(reinterpret_cast<const char*>(my_ram), ram_size);
        ram_os.close();
    }

    std::ofstream reg_os("oric_registers.snapshot", std::ios::binary);
    if (!reg_os.is_open()) return;

    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    for (auto &item : items.table) {
        std::string name = item.first;
        size_t idx = 0;

        for (auto &flag : item.second) {
            size_t current_idx = idx++; // Öka index säkert direkt för vektorn

            // HÄR FILTRERAR VI BORT ALLT SKRÄP FÖR GOTT:
            if (flag.type != 0) continue; 
            if (flag.curr == nullptr || flag.width == 0) continue;
            
            // Blockera feting-ROM:et och dolda lookup-tabeller baserat på storlek och namn
            if (flag.width > 4) continue;
            if (name.find("rom") != std::string::npos || name.find("ROM") != std::string::npos) continue;
            
            // Blockera kombinatoriska nät (flagga 8)
            if (flag.flags & (1 << 3)) continue; 

            // Det som överlever hit är enbart de små, äkta klockade hårdvaruregistren!
            uint32_t name_len = static_cast<uint32_t>(name.size());
            reg_os.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
            reg_os.write(name.data(), name_len);
            
            uint32_t saved_idx = static_cast<uint32_t>(current_idx);
            reg_os.write(reinterpret_cast<const char*>(&saved_idx), sizeof(saved_idx));

            size_t bytes_to_write = flag.width * sizeof(cxxrtl::chunk_t);
            reg_os.write(reinterpret_cast<const char*>(flag.curr), bytes_to_write);
        }
    }
    reg_os.close();
    std::cout << "[SNAPSHOT] Register sparade stabilt!" << std::endl;
}



// ============================================================================
// 2. SKOTTSÄKER NAMNBASERAD INLÄSNINGSFUNKTION (Helt immun mot förskjutningar)
// ============================================================================
bool load_system_snapshot(cxxrtl_design::p_oricatmostop &top, void* my_ram, size_t ram_size) {
    // 1. Återställ ditt externa C++ RAM-minne (oric_ram.bin)
    std::ifstream ram_is("oric_ram.bin", std::ios::binary);
    if (ram_is.is_open()) {
        ram_is.read(reinterpret_cast<char*>(my_ram), ram_size);
        ram_is.close();
        std::cout << "[SNAPSHOT] oric_ram.bin inläst och återställd." << std::endl;
    } else {
        std::cerr << "[SNAPSHOT-FEL] Avbryter: Kunde inte hitta oric_ram.bin!" << std::endl;
        return false;
    }

    // 2. Öppna register-snapshoten
    std::ifstream reg_is("oric_registers.snapshot", std::ios::binary);
    if (!reg_is.is_open()) {
        std::cerr << "[SNAPSHOT-FEL] Avbryter: Kunde inte hitta oric_registers.snapshot!" << std::endl;
        return false;
    }

    // Hämta aktuella debug_items och scopes från din design
    cxxrtl::debug_items items;
    cxxrtl::debug_scopes scopes;
    top.debug_info(&items, &scopes, "");

    size_t signals_loaded = 0;

    // Läs binärfilen linjärt tills den tar slut
    while (reg_is.peek() != EOF) {
        // A. LÄS METADATA (Exakt samma ordning som de sparades)
        uint32_t name_len = 0;
        reg_is.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        
        std::string name(name_len, '\0');
        reg_is.read(&name[0], name_len); // Säkert skriv till strängens interna buffert

        uint32_t saved_idx = 0;
        reg_is.read(reinterpret_cast<char*>(&saved_idx), sizeof(saved_idx));

        // B. SLÅ UPP UTIFRÅN STRÄNGNAMNET DIREKT
        auto it = items.table.find(name);
        if (it != items.table.end() && saved_idx < it->second.size()) {

	  std::cout << "hanterar: " << it->first << std::endl;
	  
            auto &flag = it->second[saved_idx];
            size_t bytes_to_read = flag.width * sizeof(cxxrtl::chunk_t);

            // C. DEN SÄKRA FILSTRÖMS-HÄMTNINGEN
            // Vi läser ALLTID data direkt från disk till signalens minnescell
            // för att garantera att filpekaren flyttar sig exakt rätt antal bytes.
            reg_is.read(reinterpret_cast<char*>(flag.curr), bytes_to_read);

            // D. FILTER EFTER INLÄSNING (Rör ALDRIG .next för dolda eller farliga nät!)
            // Om signalen råkar ha en trasig pekare, är kombinatorisk (flagga 8),
            // eller tillhör det feta ROM-blocket, så skippar vi att röra registerskuggan.
            if (flag.curr == nullptr || flag.width == 0 || flag.width > 4 ||
                name.find("rom") != std::string::npos || name.find("ROM") != std::string::npos ||
                (flag.flags & (1 << 3))) 
            {
                // Signalen är fredad på hårdvarunivå. Vi går tryggt vidare till nästa rad i filen.
                continue; 
            }

            // E. REGISTERSYNK (Enbart för äkta, rörliga register/flip-flops)
            // Tvinga registerskuggan (.next) att ta samma värde som vi precis laddade.
            if (flag.next && flag.next != flag.curr) {
                for (size_t i = 0; i < flag.width; ++i) {
                    flag.next[i] = flag.curr[i];
                }
            }
            signals_loaded++;
        } else {
            // Om filen innehåller en signal som strukits i RTL, har vi ett allvarligt synkfel
            std::cerr << "[SNAPSHOT-FEL] Desynk! Hittade sparad signal som saknas i nuvarande RTL: " << name << std::endl;
            reg_is.close();
            return false;
        }
    }
    reg_is.close();
    
    // 3. ÅTERSTÄLL ALLA KOMBINATORISKA NÄT NATURLIGT
    // Nu när alla register (flip-flops) har fått sina korrekta värden, 
    // låter vi CXXRTL räkna ut alla klockor och ledningar (som s_tape_byte_enable)
    top.eval(); 
    
    std::cout << "[SNAPSHOT] Återställning klar! Laddade " << signals_loaded << " äkta register framgångsrikt." << std::endl;
    return true;
}




uint32_t* find_address_by_debug_table(cxxrtl::debug_items& items, string key)
{

  cout << "Searching for signal: " << key << endl;
  
  // 1. Sök efter adressbussen i samma tabell
  auto it = items.table.find(key); // Justera namnet till er signal

  if (it != items.table.end()) {
    const cxxrtl::debug_item &addr_item = it->second.front();

    // 2. För en vanlig signal (wire) pekar .curr direkt på råa uint32_t-block
    // Vi behöver inte göra en reinterpret_cast till en hel array, bara läsa första blocket
    uint32_t *raw_data = addr_item.curr;
    return raw_data;
  }
  return nullptr;
}

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


