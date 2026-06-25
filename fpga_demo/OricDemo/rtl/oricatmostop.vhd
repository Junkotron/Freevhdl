LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY oricatmostop IS
	PORT (
		atest : out std_logic; 
		CLK_24MHz : IN STD_LOGIC; 
		RESET : in std_logic;     

		-- ========================================================================
		-- DET RENA C++ RAM-GRÄNSSNITTET (Dessa slår du upp med .lookup() i C++!)
		-- ========================================================================
		ram_ad_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		ram_d_out  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		ram_q_in   : IN  STD_LOGIC_VECTOR(7 DOWNTO 0) := (others => '0');
		ram_we_out : OUT STD_LOGIC;

		VIDEO_CLK : OUT STD_LOGIC;
		VIDEO_R : OUT STD_LOGIC;
		VIDEO_G : OUT STD_LOGIC;
		VIDEO_B : OUT STD_LOGIC;
		VIDEO_HSYNC : OUT STD_LOGIC;
		VIDEO_VSYNC : OUT STD_LOGIC
	);
END;

ARCHITECTURE RTL OF oricatmostop IS

  -- De heliga interna signalerna (Mellanlandningen som GHDL kräver!)
  signal s_ram_ad : STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal s_ram_cs : STD_LOGIC;
  signal s_ram_oe : STD_LOGIC;
  signal s_ram_we : STD_LOGIC;
  signal s_ram_d : STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal s_ram_q : STD_LOGIC_VECTOR(7 DOWNTO 0);

  signal s_via_snap_t2c_data    : STD_LOGIC_VECTOR(15 DOWNTO 0);

BEGIN

  atest <= VIDEO_R;
  s_via_snap_t2c_data <= (others => '0');

  oric : entity work.oricatmos
    port map (
		CLK_IN => CLK_24MHZ,
		RESET => RESET,
		rom => "00", 
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
		tape_byte_enable => '0',

		via_snap_ifr_data => (others => '0'),
		joystick_adapter => (others => '0'),
		joystick_0 => (others => '0'),
		joystick_1 => (others => '0'),
		bios_din => (others => '0'),
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
                
		-- Moderkortet pratar enbart med de interna signalerna!
		ram_ad => s_ram_ad,
		ram_d  => s_ram_d,
		ram_q  => s_ram_q,
		ram_we => s_ram_we,
		ram_cs => s_ram_cs,
		ram_oe => s_ram_oe
	);

  -- ========================================================================
  -- LAGLIGA TILLDELNINGAR (Här sker ping-pong-länkningen ut till C++)
  -- ========================================================================
  ram_ad_out <= s_ram_ad;
  ram_d_out  <= s_ram_d;
  ram_we_out <= s_ram_we;
  s_ram_q    <= ram_q_in; 

END RTL;
