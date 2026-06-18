library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_48k is
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
    signal ram : ram_type := (others => (others => '0'));
begin
    process(clk)
        variable addr_int     : integer;
        -- Denna variabel garanterar för CXXRTL att indexet ALDRIG kan bli för stort
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
                    addr_clamped := 0; -- Säkert standardvärde om adressen är utanför 48 kB
                end if;
                
                -- Utför skrivning (endast om adressen är giltig och we är aktiv)
                if we = '1' and addr_int <= 49151 then
                    ram(addr_clamped) <= di;
                end if;
                
                -- Synkron läsning (nu helt immun mot CXXRTL out-of-bounds)
                if addr_int <= 49151 then
                    do <= ram(addr_clamped);
                else
                    do <= (others => '0'); -- Returnera nollor för det övre området (t.ex. ROM-area)
                end if;
            else
                -- Om adressbussen är odefinierad vid start, sätt utgången till noll
                do <= (others => '0');
            end if;
        end if;
    end process;
end rtl;
