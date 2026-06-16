--
-- A simulation model of ORIC ATMOS hardware
-- Copyright (c) SEILEBOST - March 2006
-- 
-- All rights reserved
--
-- Redistribution and use in source and synthezised forms, with or without
-- modification, are permitted provided that the following conditions are met:
--
-- Redistributions of source code must retain the above copyright notice,
-- this list of conditions and the following disclaimer.
--
-- Redistributions in synthesized form must reproduce the above copyright
-- notice, this list of conditions and the following disclaimer in the
-- documentation and/or other materials provided with the distribution.
--
-- Neither the name of the author nor the names of other contributors may
-- be used to endorse or promote products derived from this software without
-- specific prior written permission.
--
-- THIS CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
-- THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
-- PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-- POSSIBILITY OF SUCH DAMAGE.
--
-- You are responsible for any legal issues arising from your use of this code.
--
-- The latest version of this file can be found at: passionoric.free.fr
--
-- Email seilebost@free.fr
--
--

--  TODO back this without clk4 divider and
--  fork to ice40 and gowik resp.

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;
ENTITY oricatmostop_sim IS
	PORT (
          CLK_TEST : out STD_LOGIC;
		CLK_EXT : IN STD_LOGIC;
		RESET_BUT1 : IN STD_LOGIC;
		key_pressed : IN STD_LOGIC;
		key_extended : IN STD_LOGIC;
		key_code : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		key_strobe : IN STD_LOGIC;
		pravetz_layout : IN STD_LOGIC := '0';
		K7_TAPEIN : IN STD_LOGIC;
		K7_TAPEOUT : OUT STD_LOGIC;
		K7_REMOTE : OUT STD_LOGIC;


		VIDEO_CLK : OUT STD_LOGIC;
		VIDEO_R : OUT STD_LOGIC;
		VIDEO_G : OUT STD_LOGIC;
		VIDEO_B : OUT STD_LOGIC;
--		VIDEO_HBLANK : OUT STD_LOGIC;
--		VIDEO_VBLANK : OUT STD_LOGIC;
		VIDEO_HSYNC : OUT STD_LOGIC;
		VIDEO_VSYNC : OUT STD_LOGIC;
		VIDEO_SYNC : OUT STD_LOGIC;
		phi2 : OUT STD_LOGIC;
		pll_locked : IN STD_LOGIC;


                -- CPU bus for external RAM
                -- d & q are handled via tristate buffer component
                ram_ad : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		ram_cs : OUT STD_LOGIC;
		ram_oe : OUT STD_LOGIC;
		ram_we : OUT STD_LOGIC;
                ram_dq : inout STD_LOGIC_VECTOR(7 DOWNTO 0)

                --;

                );
END;

ARCHITECTURE RTL OF oricatmostop_sim IS


-- Connected via tristate/bidirectional buffer 
signal ram_d : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal ram_q : STD_LOGIC_VECTOR(7 DOWNTO 0);
  
signal RESET : std_logic;

signal S_RESLOG : std_logic;
  
signal s_tape_byte_enable : STD_LOGIC;

signal s_via_snap_t2c_data    : STD_LOGIC_VECTOR(15 DOWNTO 0);

signal CLK_25MHZ : STD_LOGIC;

  -- TODO tristate impl via muxes for simulation
-- component tristate
--   port(
--     clk : in std_logic;
--     dir : in std_logic;
--     o : out std_logic_vector(7 downto 0);
--     i : in std_logic_vector(7 downto 0);
--     buff : inout std_logic_vector(7 downto 0)
--     );
-- end COMPONENT;

BEGIN
  CLK_TEST <= CLK_EXT;


  reslog: entity work.startup_reset(rtl)
    port map(
      CLK_25MHz,
      S_RESLOG
    );
          
  RESET <= not S_RESLOG or not RESET_BUT1;
  
  s_tape_byte_enable <= '0';
  s_via_snap_t2c_data <= (others => '0');

  -- TODO tristate impl via muxes for simulation
  
  -- inst_tribuf : tristate
  --   port MAP(
  --     clk => CLK_25MHz,
  --     dir => ram_oe,
  --     o => ram_q,
  --     i => ram_d,
  --     buff => ram_dq
  --     );

  
  divn_clk: entity work.clock_divider_n(Behavioral)
    generic map (
      N => 4
    )
    port map (
      clk_in => CLK_EXT,
      reset => '0',
      clk_out => CLK_25MHZ
    );

  oric : entity work.oricatmos(RTL)
    port map (
		CLK_IN => CLK_25MHZ,
		RESET => RESET,
		key_pressed => key_pressed,
		key_extended => key_extended,
		key_code => key_code,
		key_strobe => key_strobe,
		pravetz_layout => pravetz_layout,
		K7_TAPEIN => K7_TAPEIN,
		K7_TAPEOUT => K7_TAPEOUT,
		K7_REMOTE => K7_REMOTE,


		VIDEO_CLK => VIDEO_CLK,
		VIDEO_R => VIDEO_R,
		VIDEO_G => VIDEO_G,
		VIDEO_B => VIDEO_B,
--		VIDEO_HBLANK => VIDEO_HBLANK,
--		VIDEO_VBLANK => VIDEO_VBLANK,
		VIDEO_HSYNC => VIDEO_HSYNC,
		VIDEO_VSYNC => VIDEO_VSYNC,
		VIDEO_SYNC => VIDEO_SYNC,
		phi2 => phi2,
		pll_locked => pll_locked,


                -- point to signals to avoid
                -- rogue pin errors in pnr now
--                tape_byte_enable => tape_byte_enable,

                
                -- Hard coded for now

                -- scalars here
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

                -- vectors
                
		via_snap_ifr_data => (others => '0'),
		joystick_adapter => (others => '0'),
		joystick_0 => (others => '0'),
		joystick_1 => (others => '0'),
                rom => (others => '0'),
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

                
                ram_ad => ram_ad,
		ram_d  => ram_d,
		ram_q  => ram_q,
		ram_cs => ram_cs,
		ram_oe => ram_oe,
		ram_we => ram_we
                );
    

END RTL;
