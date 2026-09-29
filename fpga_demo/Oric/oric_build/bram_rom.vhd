library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_rom is
    generic (
        ADDR_WIDTH       : integer := 16;           -- Skalbar adressbredd
        RAM_DEPTH        : integer := 65536         -- Full 64KB rymd (eller 16384 för 16KB)
    );
    port (
        clk               : in  std_logic;
        
        -- Normalt Systemgränssnitt (Läsning under drift)
        addr              : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        do                : out std_logic_vector(7 downto 0);
        
        -- Extra signaler för extern PRE-LOAD (Styrs från C++ via CXXRTL)
        rom_preload_we    : in  std_logic;
        rom_preload_addr  : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        rom_preload_di    : in  std_logic_vector(7 downto 0)
    );
end bram_rom;

architecture rtl of bram_rom is
    signal internal_we   : std_logic;
    signal internal_addr : std_logic_vector(ADDR_WIDTH-1 downto 0);
    signal internal_di   : std_logic_vector(7 downto 0);
begin

    -- Multiplexer: Om vi förladdar, ge skrivsignalerna full kontroll
    internal_we   <= rom_preload_we;
    internal_addr <= rom_preload_addr when rom_preload_we = '1' else addr;
    internal_di   <= rom_preload_di   when rom_preload_we = '1' else (others => '0');

    -- 🎯 RÄTTAT: Nu instansierar vi din skottsäkra generiska RAM-modul!
    inst_rom_storage: entity work.bram_generic
        generic map (
            ADDR_WIDTH       => ADDR_WIDTH,
            RAM_DEPTH        => RAM_DEPTH,
            SIM_TEST_PATTERN => false  -- Vi vill ha tomt kisel innan C++ kör preload
        )
        port map (
            clk  => clk,
            we   => internal_we,
            addr => internal_addr,
            di   => internal_di,
            do   => do
        );
    
end rtl;
