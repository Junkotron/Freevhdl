#include "oric_rig.hpp"

// 🔥 Isolerat CXXRTL-smuts
#include <cxxrtl/cxxrtl.h>
#include "oricatmos_sim.h" 

// Definitionen av vår dolda brygga
struct OricContext {
    std::unique_ptr<cxxrtl_design::p_oricatmostop> top;
    OricContext() {
        top = std::make_unique<cxxrtl_design::p_oricatmostop>();
    }
};

// =================================================================
// 🕵️‍♂️ LOKALA HJÄLPMETODER (Bara synliga inuti oric_rig.cpp)
// =================================================================
// Om Yosys muterar ett namn i framtiden, ändrar du BARA i dessa statics!

// reset and master clock

static void set_reset_line(OricContext* ctx, bool val) {
    ctx->top->p_RESET.set<bool>(val);
}

static void set_master_clk(OricContext* ctx, bool val) {
    ctx->top->p_CLK__24MHz.set<bool>(val);
}

// cpu monitoring

static uint64_t read_cpu_regs(OricContext* ctx) {
    return ctx->top->p_top__cpu__regs.get<uint64_t>();
}

static bool read_sync_signal(OricContext* ctx) {
    return ctx->top->p_top__cpu__sync.get<bool>();
}

static bool read_ram_we(OricContext* ctx) {
    return ctx->top->p_top__ram__we.get<bool>();
}

static bool read_phi2_clk(OricContext* ctx) {
    return ctx->top->p_top__phi2.get<bool>();
}

static uint8_t read_dbus(OricContext* ctx) {
    return ctx->top->p_top__cpu__dbus.get<uint8_t>();
}

// via 6522 testing

static bool read_via_irq(OricContext* ctx) {
    return ctx->top->p_oureasytofindinterrupt.get<bool>();
}

// generic rom handling

static void set_top_rom_select(OricContext* ctx, uint8_t val) {
    ctx->top->p_top__rom__select.set<uint8_t>(val);
}

static void set_rom_preload_we(OricContext* ctx, bool val) {
    ctx->top->p_rom__preload__we.set<bool>(val);
}

static void set_rom_preload_addr(OricContext* ctx, uint16_t addr) {
    ctx->top->p_rom__preload__addr.set<uint16_t>(addr);
}

static void set_rom_preload_di(OricContext* ctx, uint8_t val) {
    ctx->top->p_rom__preload__di.set<uint8_t>(val);
}


// =================================================================
// 👑 PUBLIKA OricRig-METODER (Det som main.cpp ser)
// =================================================================

OricRig::OricRig() {
    ctx = new OricContext();
}

OricRig::~OricRig() {
    delete ctx;
}

void OricRig::hardware_reset() {
    // Elegant, läsbart och helt fritt från rått ctx-tugg!
    set_reset_line(ctx, true);
    
    for (int i = 0; i < 48; i++) {
        set_master_clk(ctx, true);
        ctx->top->step();
        set_master_clk(ctx, false);
        ctx->top->step();
    }
    
    set_reset_line(ctx, false);
}

void OricRig::clock_master_high() {
    set_master_clk(ctx, true);
    ctx->top->step();
}

void OricRig::clock_master_low() {
    set_master_clk(ctx, false);
    ctx->top->step();
}

uint64_t OricRig::get_cpu_regs() const {
    return read_cpu_regs(ctx);
}

bool OricRig::is_sync() const {
    return read_sync_signal(ctx);
}

bool OricRig::get_we() const {
    return read_ram_we(ctx);
}

bool OricRig::get_cpu_clk() const {
    return read_phi2_clk(ctx);
}

uint8_t OricRig::get_bus_data() const {
    return read_dbus(ctx);
}

uint16_t OricRig::get_pc() const {
    return (read_cpu_regs(ctx) >> 48) & 0xFFFF;
}

bool OricRig::get_via_irq() const {
    return read_via_irq(ctx);
}

void OricRig::set_rom_select(uint8_t mode) {
    set_top_rom_select(ctx, mode);
}

// 💡 DEN NYA SKOTTSÄKRA INLADDNINGEN!
bool OricRig::load_rom_image(const std::string& filename, uint16_t start_addr) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "❌ [OricRig] Kunde inte öppna ROM-filen: " << filename << std::endl;
        return false;
    }

    // Läs in hela filen till en tillfällig vektor
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "📥 [OricRig] Förladdar " << std::dec << buffer.size() << " bytes till $" 
              << std::hex << std::uppercase << start_addr << " via VHDL-bussen..." << std::endl;

    // Aktivera förladdnings-gränssnittet i hårdvaran
    set_rom_preload_we(ctx, true);
    ctx->top->step();
    
    // Klocka in varenda byte sekventiellt
    for (size_t i = 0; i < buffer.size(); i++) {
        uint16_t current_target = start_addr + i;
        
        set_rom_preload_addr(ctx, current_target);
        set_rom_preload_di(ctx, buffer[i]);

        // Trigger stigande kant på masterklockan så att din bram_48k nyper byten
	ctx->top->step();
	
        clock_master_high();
        clock_master_low();
    }

    // Koppla ur förladdningen och återställ bussarna till normalläge
    set_rom_preload_we(ctx, false);
    set_rom_preload_addr(ctx, 0);
    set_rom_preload_di(ctx, 0);

    ctx->top->step();
    
    // Ett litet extra tomsteg för att stabilisera signalerna
    clock_master_high(); clock_master_low();

    std::cout << "✨ [OricRig] ROM-avbildningen inspelad på kisel!" << std::endl;
    return true;
}

