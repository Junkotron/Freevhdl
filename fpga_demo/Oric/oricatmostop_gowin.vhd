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


LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;
ENTITY oricatmostop_gowin IS
	PORT (
                atest : out std_logic; -- it is usuful in the real world wo we
                                       -- just let it tagalong here
            
                CLK_24MHz : IN STD_LOGIC; -- old mister clock
                RESET : in std_logic;     -- magic reset from sim

		VIDEO_CLK : OUT STD_LOGIC;
		VIDEO_R : OUT STD_LOGIC;
		VIDEO_G : OUT STD_LOGIC;
		VIDEO_B : OUT STD_LOGIC;
		VIDEO_HSYNC : OUT STD_LOGIC;
		VIDEO_VSYNC : OUT STD_LOGIC

                -- ;
--		phi2 : OUT STD_LOGIC;
--		pll_locked : IN STD_LOGIC
                -- ;

                );
END;

ARCHITECTURE RTL OF oricatmostop_gowin IS

  
  -- CPU bus for external (and extended) RAM
  -- d & q are handled via tristate buffer component
  signal s_ram_ad : STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal s_ram_cs : STD_LOGIC;
  signal s_ram_oe : STD_LOGIC;
  signal s_ram_we : STD_LOGIC;

  signal s_ram_d : STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal s_ram_q : STD_LOGIC_VECTOR(7 DOWNTO 0);


  signal s_tape_byte_enable : STD_LOGIC;

  signal s_via_snap_t2c_data    : STD_LOGIC_VECTOR(15 DOWNTO 0);

BEGIN

  atest <= VIDEO_R;
  
  s_via_snap_t2c_data <= (others => '0');

  oric : entity work.oricatmos(RTL)
    port map (
		CLK_IN => CLK_24MHZ,
		RESET => RESET,

                rom => "01", -- nice and shiny just out of the plastic oric 1.1 rom

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

                
                ram_ad => s_ram_ad,
		ram_d  => s_ram_d,
		ram_q  => s_ram_q,
		ram_we => s_ram_we,

                -- not actually used for bram
		ram_cs => s_ram_cs,
		ram_oe => s_ram_oe

                );
  

  inst_oricram: entity work.bram_48k(rtl)
    generic map (
      SIM_TEST_PATTERN => true
      )
    port map (
      clk  => clk_24MHz,     -- Din 24 MHz masterklocka
      we   => s_ram_we,
      addr => s_ram_ad,
      di   => s_ram_d,    -- Data från CPU:ns DO-pinne
      do   => s_ram_q -- Data ut till multiplexern
      );
  
END RTL;
