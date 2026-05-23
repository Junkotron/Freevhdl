library ieee;
	use ieee.std_logic_1164.all;
	use ieee.std_logic_unsigned.all;

--      ULA pinout
--  1 MUX    U RAM_D1 40
--  2 RAM_D2   RAM_D0 39
--  3 RAM_D3   RAM_D7 38
--  4 RAM_D4   RAM_D5 37
--  5 D5       RAM_D6 36
--  6 GND         A12 35
--  7 CLK          D6 34
--  8 D0          A09 33
--  9 CAS         A08 32
-- 10 RAS         A10 31
-- 11 D2          A15 30
-- 12 D3          A14 29
-- 13 D4      RAM_R/W 28
-- 14 PHI         R/W 27
-- 15 A11         MAP 26
-- 16 SYNC        I/O 25
-- 17 D1          Vcc 24
-- 18 D7       ROM_CS 23
-- 19 BLU         A13 22
-- 20 GRN         RED 21



entity ulatop is
port (
	RESETn     :   in  std_logic;                    
	CLK_4      :   out std_logic;                    
	CLK_4_EN   :   out std_logic;                    

	CLK        :   in  std_logic;                    
	PHI2       :   out std_logic;                    
	PHI2_EN    :   out std_logic;                    
	RW         :   in  std_logic;                    
	MAPn       :   in  std_logic;                    
        CSIOn      :   out std_logic;                    
	CSROMn     :   out std_logic;                    
	R          :   out std_logic;                    
	G          :   out std_logic;                    
	B          :   out std_logic;                    
	SYNC       :   out std_logic;                    

        DBTOP         :   in  std_logic_vector( 7 downto 0);
	ADDRHI     :   in  std_logic_vector(15 downto 8)
);
end;

architecture RTL of ulatop is
begin
  ULA_1 : entity work.ula(RTL)
    port map (
	RESETn => RESETn,
	CLK_4 => CLK_4,
	CLK_4_EN => CLK_4_EN,
	CLK => CLK,
	PHI2 => PHI2,
	PHI2_EN => PHI2_EN,
	RW => RW,
	MAPn => MAPn,

        CSIOn => CSIOn,
	CSROMn => CSROMn,
	R => R,
	G => G,
	B => B,
	SYNC => SYNC,
        
        ADDR(15 downto 8) => ADDRHI,
        ADDR(7 downto 0) => (others => '0'),
        DB => DBTOP
        
    );
  
end architecture RTL;
