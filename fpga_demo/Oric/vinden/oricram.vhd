library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_48k is
    generic (
        -- Sätt till true i din testbench/simulator för att ladda schackrutan
        -- brasklapp: detta funkar på dom flesta moderna syntesverktyg men
        -- kan få för sig att sluka gindar i vissa system
        -- I cxxrtl måste man komplettera med en init metod i cpp-koden
        SIM_TEST_PATTERN : boolean := false 
    );
    port (
        clk  : in  std_logic;
        we   : in  std_logic;                    -- Write Enable (Aktiv hög)
        addr : in  std_logic_vector(15 downto 0); -- 16-bitars adressbuss
        di   : in  std_logic_vector(7 downto 0);  -- Data In till RAM
        do   : out std_logic_vector(7 downto 0)   -- Data Ut från RAM
    );
end bram_48k;

architecture rtl of bram_48k is
    -- 48 kB RAM motsvarar index 0 till 49151
    type ram_type is array (0 to 49151) of std_logic_vector(7 downto 0);

    -- Intern funktion som väljer mellan nollställt RAM eller schackrutat mönster
    function init_ram_data(enable_pattern : boolean) return ram_type is
        variable temp_ram : ram_type;
    begin
        for i in 0 to 49151 loop
            if enable_pattern then
                -- Skapar ett mönster: 0x55 (01010101) på jämna adresser, 0xAA (10101010) på udda
                if (i mod 2 = 0) then
                    temp_ram(i) := x"55";
                else
                    temp_ram(i) := x"AA";
                end if;
            else
                -- Standard för hårdvara: Fyll med nollor
                temp_ram(i) := x"00";
            end if;
        end loop;
        return temp_ram;
    end function;

    -- Initiera minnet dynamiskt baserat på vår generic
    signal ram : ram_type := init_ram_data(SIM_TEST_PATTERN);

begin
    process(clk)
        variable addr_int     : integer;
        variable addr_clamped : integer range 0 to 49151; 
    begin
        if rising_edge(clk) then
            -- Skydda simulatorn mot oinitierade värden ('U', 'X') vid boot
            if not is_X(addr) then
                addr_int := to_integer(unsigned(addr));
                
                -- Sätt en stenhård gräns för array-indexeringen
                if addr_int <= 49151 then
                    addr_clamped := addr_int;
                else
                    addr_clamped := 0; 
                end if;
                
                -- Utför skrivning (endast om adressen är giltig och we är aktiv)
                if we = '1' and addr_int <= 49151 then
                    ram(addr_clamped) <= di;
                end if;
                
                -- Synkron läsning (helt immun mot CXXRTL out-of-bounds)
                if addr_int <= 49151 then
                    do <= ram(addr_clamped);
                else
                    do <= (others => '0'); 
                end if;
            else
                do <= (others => '0');
            end if;
        end if;
    end process;
end rtl;
