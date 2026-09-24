library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_48k is
    generic (
        SIM_TEST_PATTERN : boolean := true -- Behåll flaggan utåt för bakåtkompatibilitet
    );
    port (
        clk  : in  std_logic;
        we   : in  std_logic;
        addr : in  std_logic_vector(15 downto 0);
        di   : in  std_logic_vector(7 downto 0);
        do   : out std_logic_vector(7 downto 0)
    );
end bram_48k;

architecture rtl of bram_48k is
begin

    -- Vi instansierar den skottsäkrade generiska kärnan under huven!
    inst_core : entity work.bram_generic
        generic map (
            ADDR_WIDTH       => 16,
            RAM_DEPTH        => 49152,          -- Äkta 48KB djup
            SIM_TEST_PATTERN => SIM_TEST_PATTERN -- Skicka vidare mönstret
        )
        port map (
            clk  => clk,
            we   => we,
            addr => addr,
            di   => di,
            do   => do
        );

end rtl;
