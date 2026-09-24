library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_rom is
    port (
        clk               : in  std_logic;
        
        -- Normalt Systemgränssnitt (Läsning under drift)
        addr              : in  std_logic_vector(15 downto 0);
        do                : out std_logic_vector(7 downto 0);
        
        -- Extra signaler för extern PRE-LOAD (Styrs från C++ via CXXRTL)
        rom_preload_we    : in  std_logic;
        rom_preload_addr  : in  std_logic_vector(15 downto 0);
        rom_preload_di    : in  std_logic_vector(7 downto 0)
    );
end bram_rom;

architecture rtl of bram_rom is
    signal internal_we   : std_logic;
    signal internal_addr : std_logic_vector(15 downto 0);
    signal internal_di   : std_logic_vector(7 downto 0);
begin

    -- Multiplexer: Om vi förladdar, ge skrivsignalerna full kontroll
    internal_we   <= rom_preload_we;
    internal_addr <= rom_preload_addr when rom_preload_we = '1' else addr;
    internal_di   <= rom_preload_di   when rom_preload_we = '1' else (others => '0');

    -- Instansiering av din skottsäkra generiska RAM-modul!
    -- Vi stänger av SIM_TEST_PATTERN då vi vill ha ett tomt minne innan preload.
    inst_rom_storage: entity work.bram_48k
        generic map (
            SIM_TEST_PATTERN => false
        )
        port map (
            clk  => clk,
            we   => internal_we,
            addr => internal_addr,
            di   => internal_di,
            do   => do
        );

end rtl;
