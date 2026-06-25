-- Cumulus CPLD Core
-- Top Level Entity (Dummy Edition för CXXRTL)
-- Copyright 2010 Retromaster

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

architecture rtl of Microdisc is
begin

	-- 1. Din fungerande reset-transfer
	nHOSTRST <= nRESET;

	-- 2. HÄV BLOCKADEN DIREKT PÅ UTGÅNGARNA:
	-- Vi tvingar ut ettor på de aktiv-låga kontrollpinnarna!
	nROMDIS <= '1'; -- '1' = Stäng INTE av interna BASIC-ROM! (Häv blockaden)
	nMAP    <= '1'; -- '1' = Slå INTE på någon extern minnesmappning!
	nIRQ    <= '1'; -- '1' = Deaktiverad avbrottslina (Aktivt låg, så '1' = tyst!)
	nOE     <= '1'; -- Output Enable inaktiv (Aktivt låg)
	nECE    <= '1'; -- Chip Enable deaktiverad (Aktivt låg)
	nEOE    <= '1'; -- Output Enable deaktiverad (Aktivt låg)

	-- 3. SOLID JORDNING AV ALLA UTGÅENDE BUSSAR OCH STATUSSIGNALER
	DO         <= (others => '0'); -- Tyst på databussen ut
	DIR        <= '0';             -- Standard riktning på bussen
	IOCTRL     <= '0';             -- Ingen I/O-kontroll aktiv

	-- SD-KORTETS CONTROL- OCH ADRESSBUSSAR JORDAS TOTALT
	sd_lba_fd0 <= (others => '0');
	sd_lba_fd1 <= (others => '0');
	sd_lba_fd2 <= (others => '0');
	sd_lba_fd3 <= (others => '0');
	sd_rd      <= (others => '0');
	sd_wr      <= (others => '0');
	sd_din_fd0 <= (others => '0');
	sd_din_fd1 <= (others => '0');
	sd_din_fd2 <= (others => '0');
	sd_din_fd3 <= (others => '0');

	-- FLOPPY-STATUSEN SÄTTS TILL TILLGÄNGLIG OCH SLÄCKT
	fdd_busy   <= '0';             -- Diskdriven är inte upptagen
	fd_led     <= '0';             -- Drive-lampan är släckt

end rtl;

