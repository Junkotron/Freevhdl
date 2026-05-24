-- Cumulus CPLD Core
-- Top Level Entity
-- Copyright 2010 Retromaster
--
-- This file is part of Cumulus CPLD Core.
--
-- Cumulus CPLD Core is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License,
-- or any later version.
--
-- Cumulus CPLD Core is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Cumulus CPLD Core. If not, see .
--

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

ENTITY Microdisc IS

	PORT (
		CLK_SYS : IN STD_LOGIC; -- 24 Mhz input clock

		-- Oric Expansion Port Signals
		DI : IN STD_LOGIC_VECTOR(7 DOWNTO 0); -- 6502 Data Bus
		DO : OUT STD_LOGIC_VECTOR(7 DOWNTO 0); -- 6502 Data Bus

		A : IN STD_LOGIC_VECTOR(15 DOWNTO 0); -- 6502 Address Bus
		RnW : IN STD_LOGIC; -- 6502 Read-/Write
		nIRQ : OUT STD_LOGIC; -- 6502 /IRQ
		PH2 : IN STD_LOGIC; -- 6502 PH2
		nROMDIS : OUT STD_LOGIC; -- Oric ROM Disable
		nMAP : OUT STD_LOGIC; -- Oric MAP
		IO : IN STD_LOGIC; -- Oric I/O
		IOCTRL : OUT STD_LOGIC; -- Oric I/O Control 
		nHOSTRST : OUT STD_LOGIC; -- Oric RESET

		-- Data Bus Buffer Control Signals
		nOE : OUT STD_LOGIC; -- Output Enable
		DIR : OUT STD_LOGIC; -- Direction
		-- Additional MCU Interface Lines
		nRESET : IN STD_LOGIC; -- RESET from MCU
		--		DSEL            : OUT std_logic_vector(1 DOWNTO 0); -- Drive Select
		--		SSEL            : OUT  std_logic; -- Side Select

		-- EEPROM Control Lines.
		nECE : OUT STD_LOGIC; -- Chip Enable
		nEOE : OUT STD_LOGIC; -- Output Enable
		ENA : IN STD_LOGIC;

		img_mounted : IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		img_wp : IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		img_size : IN STD_LOGIC_VECTOR (31 DOWNTO 0);
		sd_lba_fd0 : OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
		sd_lba_fd1 : OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
		sd_lba_fd2 : OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
		sd_lba_fd3 : OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
		sd_rd : OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		sd_wr : OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		sd_ack : IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		sd_buff_addr : IN STD_LOGIC_VECTOR (8 DOWNTO 0);
		sd_dout : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		sd_din_fd0 : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		sd_din_fd1 : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		sd_din_fd2 : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		sd_din_fd3 : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
		sd_dout_strobe : IN STD_LOGIC;
		sd_din_strobe : IN STD_LOGIC;

		fdd_ready : IN STD_LOGIC;
		fdd_busy : BUFFER STD_LOGIC;
		fdd_reset : IN STD_LOGIC;
		fdd_layout : IN STD_LOGIC;
		fd_led : OUT STD_LOGIC
	);
END Microdisc;

ARCHITECTURE Behavioral OF Microdisc IS
BEGIN
  
END Behavioral;
