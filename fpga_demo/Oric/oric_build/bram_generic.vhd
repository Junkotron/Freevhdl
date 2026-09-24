library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_generic is
    generic (
        ADDR_WIDTH       : integer := 16;           -- Antal adressbitar (t.ex. 16 för 48KB/64KB)
        RAM_DEPTH        : integer := 49152;        -- Totalt antal bytes (48KB = 49152)
        SIM_TEST_PATTERN : boolean := true          -- Det klassiska 55/AA mönstret
    );
    port (
        clk  : in  std_logic;
        we   : in  std_logic;                       -- Write Enable
        addr : in  std_logic_vector(ADDR_WIDTH-1 downto 0); -- Skalbar adressbuss
        di   : in  std_logic_vector(7 downto 0);    -- Data In
        do   : out std_logic_vector(7 downto 0)     -- Data Ut
    );
end bram_generic;

architecture rtl of bram_generic is
    -- 💡 KISELMATRISEN: Nu helt skalbar baserad på max-djupet
    type ram_type is array (0 to RAM_DEPTH-1) of std_logic_vector(7 downto 0);

    -- Din original-funktion för testmönster, helt parametriserad!
    function init_ram_data(enable_pattern : boolean) return ram_type is
        variable temp_ram : ram_type;
    begin
        for i in 0 to RAM_DEPTH-1 loop
            if enable_pattern then
                if (i mod 2 = 0) then temp_ram(i) := x"55";
                else                  temp_ram(i) := x"AA";
                end if;
            else
                temp_ram(i) := x"00";
            end if;
        end loop;
        return temp_ram;
    end function;

    -- CXXRTL kommer att hitta denna matris under namnet "ram" precis som i originalet!
    signal ram : ram_type := init_ram_data(SIM_TEST_PATTERN);

begin

    -- DIN RENA SYNKRONA PROCESSPASSNING MED KRASCHSKYDD (Helt orörd i logiken!)
    process(clk)
        variable addr_unsigned : unsigned(ADDR_WIDTH-1 downto 0);
        variable addr_int      : integer;
        variable safe_index    : integer range 0 to RAM_DEPTH-1; -- Skalbart kraschskydd
    begin
        if rising_edge(clk) then
            if not is_X(addr) then
                addr_unsigned := unsigned(addr);
                addr_int      := to_integer(addr_unsigned);
                
                -- Tvinga fram ett stenhårt index som CXXRTL:s kompilator kan validera
                if addr_int <= (RAM_DEPTH-1) then
                    safe_index := addr_int;
                else
                    safe_index := 0; -- Om adresserat minne är out of bounds, peka på cell 0
                end if;

                -- Utför skrivning och läsning via safe_index på masterklockan
                if addr_int <= (RAM_DEPTH-1) then
                    if we = '1' then
                        ram(safe_index) <= di;
                    end if;
                    do <= ram(safe_index);
                else
                    do <= (others => '0'); -- Returnera noll för adresser utanför matrisen
                end if;
            else
                do <= (others => '0');
            end if;
        end if;
    end process;

end rtl;
