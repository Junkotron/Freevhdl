LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY oricatmostop IS
  PORT (
    atest : out std_logic;
    CLK_24MHz : IN STD_LOGIC;
    RESET : in std_logic;
    VIDEO_CLK : OUT STD_LOGIC;
    VIDEO_R : OUT STD_LOGIC;
    VIDEO_G : OUT STD_LOGIC;
    VIDEO_B : OUT STD_LOGIC;
    VIDEO_HSYNC : OUT STD_LOGIC;
    VIDEO_VSYNC : OUT STD_LOGIC;
    
    -- 🕵️‍♂️ VÅRT NYA UNDERSTREKSFRIA CHARTER:
    oureasytofindinterrupt : out std_logic;

    -- DEBUG STUFF
    top_cpu_regs : out std_logic_vector(63 DOWNTO 0);
    top_phi2     : out std_logic;
    top_ram_we   : out std_logic;
    top_cpu_dbus : out std_logic_vector(7 DOWNTO 0);
    top_cpu_sync : out std_logic;

    -- 💾 NY PORT FÖR ATT DEKLAREMA OCH STYRA ROM-LÄGE UTIFRÅN
    top_rom_select : in std_logic_vector(1 DOWNTO 0) := "00";
    
    -- 💡 DE EXTERNA PORTARNA FÖR CXXRTL / C++
    rom_preload_we   : in  std_logic := '0';
    rom_preload_addr : in  std_logic_vector(15 DOWNTO 0) := (others => '0');
    rom_preload_di   : in  std_logic_vector(7 DOWNTO 0) := (others => '0')


    );
END;

ARCHITECTURE RTL OF oricatmostop IS

  signal s_ram_ad : STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal s_ram_cs : STD_LOGIC;
  signal s_ram_oe : STD_LOGIC;
  signal s_ram_we : STD_LOGIC;
  signal s_ram_d : STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal s_ram_q : STD_LOGIC_VECTOR(7 DOWNTO 0);

  signal s_tape_byte_enable : STD_LOGIC;
  signal s_via_snap_t2c_data    : STD_LOGIC_VECTOR(15 DOWNTO 0);

  signal s_phi2        : STD_LOGIC;
  signal s_via_snap_q  : STD_LOGIC_VECTOR(136 DOWNTO 0);
  -- 💡 LIVFÖRSÄKRINGEN: Här deklarerar vi den sanna avbrottstråden!
  signal s_via_irq_n   : STD_LOGIC; 

  -- 💡 INTERNA PRELOAD-SIGNALER
  signal s_rom_preload_we   : std_logic;
  signal s_rom_preload_addr : std_logic_vector(15 downto 0);
  signal s_rom_preload_di   : std_logic_vector(7 downto 0);
  
  -- 💡 NY INTERN DATAUT-SIGNAL FRÅN ROM-MODULEN
  signal s_rom_q            : std_logic_vector(7 downto 0);

  -- Berätta för GHDL exakt hur MikeJ:s VIA ser ut
  component M6522 is
     port (
        I_RS                  : in    std_logic_vector(3 downto 0);
        I_DATA                : in    std_logic_vector(7 downto 0);
        O_DATA                : out   std_logic_vector(7 downto 0);
        O_DATA_OE_L           : out   std_logic;
        I_RW_L                : in    std_logic;
        I_CS1                 : in    std_logic;
        I_CS2_L               : in    std_logic;
        O_IRQ_L               : out   std_logic;
        I_CA1                 : in    std_logic;
        I_CA2                 : in    std_logic;
        O_CA2                 : out   std_logic;
        O_CA2_OE_L            : out   std_logic;
        I_PA                  : in    std_logic_vector(7 downto 0);
        O_PA                  : out   std_logic_vector(7 downto 0);
        O_PA_OE_L             : out   std_logic_vector(7 downto 0);
        I_CB1                 : in    std_logic;
        O_CB1                 : out   std_logic;
        O_CB1_OE_L            : out   std_logic;
        I_CB2                 : in    std_logic;
        O_CB2                 : out   std_logic;
        O_CB2_OE_L            : out   std_logic;
        I_PB                  : in    std_logic_vector(7 downto 0);
        O_PB                  : out   std_logic_vector(7 downto 0);
        O_PB_OE_L             : out   std_logic_vector(7 downto 0);
        I_P2_H                : in    std_logic;
        RESET_L               : in    std_logic;
        ENA_4                 : in    std_logic;
        CLK                   : in    std_logic;
        snap_we               : in    std_logic := '0';
        snap_addr             : in    std_logic_vector(3 downto 0) := (others => '0');
        snap_data             : in    std_logic_vector(7 downto 0) := (others => '0');
        snap_t1c_we           : in    std_logic := '0';
        snap_t1c_data         : in    std_logic_vector(15 downto 0) := (others => '0');
        snap_t2c_we           : in    std_logic := '0';
        snap_t2c_data         : in    std_logic_vector(15 downto 0) := (others => '0');
        snap_t_active_we      : in    std_logic := '0';
        snap_t1_active        : in    std_logic := '0';
        snap_t2_active        : in    std_logic := '0';
        snap_ifr_we           : in    std_logic := '0';
        snap_ifr_data         : in    std_logic_vector(6 downto 0) := (others => '0');
        snap_q                : out   std_logic_vector(136 downto 0)
     );
  end component;
  
BEGIN

  -- ⚡ Bind ihop spionpinnen med VIA:ns avbrottsutgång!
  oureasytofindinterrupt <= s_via_irq_n;
  
  atest <= '1' when top_cpu_regs(63 downto 48) = x"C00F" else '0';
  top_phi2 <= s_phi2;
  top_ram_we <= s_ram_we;
  s_via_snap_t2c_data <= (others => '0');

  
  oric : entity work.oricatmos(RTL)
    port map (
		CLK_IN => CLK_24MHz,
		RESET => RESET,
		rom => top_rom_select,
		key_pressed => '0',
		key_extended => '0',
		key_code => "00000000",
		key_strobe => '0',
		pravetz_layout => '0',
		K7_TAPEIN => '0',
		VIDEO_CLK => VIDEO_CLK,
		VIDEO_R => VIDEO_R,
		VIDEO_G => VIDEO_G,
		VIDEO_B => VIDEO_B,
		VIDEO_HSYNC => VIDEO_HSYNC,
		VIDEO_VSYNC => VIDEO_VSYNC,
		pll_locked => not RESET,
                
		cpu_regs_q => top_cpu_regs,
		cpu_dbus_debug => top_cpu_dbus,
		cpu_sync_out => top_cpu_sync,
                
		fdd_ready => '0',
		fdd_reset => '0',
		fdd_layout => '0',
		disk_enable => '0',
		patch_active    => '0',
		sd_dout_strobe => '0',
		sd_din_strobe => '0',
		cpu_halt => '0',
		cpu_regs_set_we => '0',
		via_snap_we     => '0',
		via_snap_t1c_we      => '0',
		via_snap_t2c_we      => '0',
		via_snap_t_active_we => '0',
		via_snap_t1_active   => '0',
		via_snap_t2_active   => '0',
		via_snap_ifr_we      => '0',
		ay_snap_we      => '0',
		ay_snap_creg_we => '0',
		ula_snap_mode_we => '0',
		tape_byte_enable => s_tape_byte_enable,

		via_snap_ifr_data => (others => '0'),
		joystick_adapter => (others => '0'),
		joystick_0 => (others => '0'),
		joystick_1 => (others => '0'),
		bios_din => s_rom_q,
		img_mounted => (others => '0'),
		img_wp => (others => '0'),
		img_size => (others => '0'),
		sd_ack => (others => '0'),
		sd_buff_addr => (others => '0'),
		sd_dout => (others => '0'),
		cpu_regs_set    => (others => '0'),
		via_snap_addr   => (others => '0'),
		via_snap_data   => (others => '0'),
		via_snap_t1c_data    => (others => '0'),
		via_snap_t2c_data    => s_via_snap_t2c_data,
		ay_snap_addr    => (others => '0'),
		ay_snap_data    => (others => '0'),
		ay_snap_creg    => (others => '0'),
		ula_snap_mode   => (others => '0'),
		patch_data      => (others => '0'),
                
		ram_ad => s_ram_ad,
		ram_d  => s_ram_d,
		ram_q  => s_ram_q,
		ram_we => s_ram_we,
		ram_cs => s_ram_cs,
		ram_oe => s_ram_oe,
		phi2   => s_phi2,
		via_snap_q => s_via_snap_q
    );

  inst_via: M6522
    port map (
      I_RS        => s_ram_ad(3 downto 0),
      I_DATA      => s_ram_d,
      O_DATA      => open,
      O_DATA_OE_L => open,
      I_RW_L      => '1',
      I_CS1       => '1',
      I_CS2_L     => '0',
      O_IRQ_L     => open,
      I_CA1       => '1',
      I_CA2       => '1',
      O_CA2       => open,
      O_CA2_OE_L  => open,
      I_PA        => (others => '0'),
      O_PA        => open,
      O_PA_OE_L   => open,
      I_CB1       => '1',
      O_CB1       => open,
      O_CB1_OE_L  => open,
      I_CB2       => '1',
      O_CB2       => open,
      O_CB2_OE_L  => open,
      I_PB        => (others => '0'),
      O_PB        => open,
      O_PB_OE_L   => open,
      I_P2_H      => s_phi2,
      RESET_L     => not RESET,
      ENA_4       => '1',
      CLK         => CLK_24MHz,
      snap_we     => '0',
      snap_addr   => (others => '0'),
      snap_data   => (others => '0'),
      snap_t1c_we => '0',
      snap_t1c_data => (others => '0'),
      snap_t2c_we => '0',
      snap_t2c_data => (others => '0'),
      snap_t_active_we => '0',
      snap_t1_active => '0',
      snap_t2_active => '0',
      snap_ifr_we => '0',
      snap_ifr_data => (others => '0'),
      snap_q      => open
    );

  inst_oricram: entity work.bram_48k(rtl)
    port map (
      clk  => CLK_24MHz,
      we   => s_ram_we,
      addr => s_ram_ad,
      di   => s_ram_d,
      do   => s_ram_q
      );

  inst_oricrom: entity work.bram_rom
    port map (
      clk              => clk_24MHz,
      addr             => s_ram_ad,
      do               => s_rom_q,
      rom_preload_we   => rom_preload_we,
      rom_preload_addr => rom_preload_addr,
      rom_preload_di   => rom_preload_di
    );

  
END RTL;
