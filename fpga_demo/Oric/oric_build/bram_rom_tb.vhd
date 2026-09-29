library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bram_rom_tb is
-- En testbank ar tom utat
end bram_rom_tb;

architecture sim of bram_rom_tb is

    -- 1. Komponentdeklaration for enheten under test (UUT)
    component bram_rom is
        generic (
            ADDR_WIDTH       : integer := 16;
            RAM_DEPTH        : integer := 65536
        );
        port (
            clk               : in  std_logic;
            addr              : in  std_logic_vector(15 downto 0);
            do                : out std_logic_vector(7 downto 0);
            rom_preload_we    : in  std_logic;
            rom_preload_addr  : in  std_logic_vector(15 downto 0);
            rom_preload_di    : in  std_logic_vector(7 downto 0)
        );
    end component;

    -- 2. Interna signaler for att driva bussen
    signal clk               : std_logic := '0';
    signal addr              : std_logic_vector(15 downto 0) := (others => '0');
    signal do                : std_logic_vector(7 downto 0);
    
    signal rom_preload_we    : std_logic := '0';
    signal rom_preload_addr  : std_logic_vector(15 downto 0) := (others => '0');
    signal rom_preload_di    : std_logic_vector(7 downto 0) := (others => '0');

    -- Klockperiod (~41.67 ns for 24 MHz masterklocka)
    constant CLK_PERIOD : time := 41.67 ns;
    signal sim_done     : boolean := false;

begin

    -- 3. Instansiera vart programmerbara ROM (Nu med 64KB konfiguration)
    uut: bram_rom
        generic map (
            ADDR_WIDTH => 16,
            RAM_DEPTH  => 65536
        )
        port map (
            clk               => clk,
            addr              => addr,
            do                => do,
            rom_preload_we    => rom_preload_we,
            rom_preload_addr  => rom_preload_addr,
            rom_preload_di    => rom_preload_di
        );

    -- 4. Klockgenerator
    clk_process : process
    begin
        while not sim_done loop
            clk <= '0';
            wait for CLK_PERIOD / 2;
            clk <= '1';
            wait for CLK_PERIOD / 2;
        end loop;
        wait;
    end process;

    -- 5. Stimulusprocessen: ASCII-renad och redo for GHDL
    stimulus : process
    begin
        -- Lat systemet stabilisera sig
        wait for CLK_PERIOD * 5;

        -----------------------------------------------------------------------
        -- FAS 1: PRE-LOAD (Motsvarar load_rom_image i C++)
        -----------------------------------------------------------------------
        report "[TB] STARTING PRE-LOAD SEQUENCE VIA CXXRTL BRIDGE...";
        rom_preload_we <= '1';
        
        -- Skriv byte 1: $AA till adress $C000
        rom_preload_addr <= X"C000";
        rom_preload_di   <= X"AA";
        wait for CLK_PERIOD; 

        -- Skriv byte 2: $BB till adress $C001
        rom_preload_addr <= X"C001";
        rom_preload_di   <= X"BB";
        wait for CLK_PERIOD;

        -- Skriv byte 3: $00 till adress $FFFC
        rom_preload_addr <= X"FFFC";
        rom_preload_di   <= X"00";
        wait for CLK_PERIOD;

        -- Skriv byte 4: $C0 till adress $FFFD
        rom_preload_addr <= X"FFFD";
        rom_preload_di   <= X"C0";
        wait for CLK_PERIOD;

        -- Aterstall forladdningsbussen till normallage
        rom_preload_we   <= '0';
        rom_preload_addr <= (others => '0');
        rom_preload_di   <= (others => '0');
        wait for CLK_PERIOD * 2;
        report "[TB] PRE-LOAD FINISHED. SWITCHING TO NORMAL OPERATION MODE.";

        -----------------------------------------------------------------------
        -- FAS 2: VERIFIERING (Normal lasning under drift)
        -----------------------------------------------------------------------
        -- Testa lasning fran adress $C000
        addr <= X"C000";
        wait for CLK_PERIOD; 
        assert do = X"AA" 
            report "[ERROR] Failed to read AA from C000!"
            severity error;

        -- Testa lasning fran adress $C001
        addr <= X"C001";
        wait for CLK_PERIOD;
        assert do = X"BB" 
            report "[ERROR] Failed to read BB from C001!"
            severity error;

        -- Testa en tom adress
        addr <= X"D000";
        wait for CLK_PERIOD;
        assert do = X"00" 
            report "[ERROR] Unexpected data found at empty address D000!"
            severity error;

        -- Kontrollera Reset-vektorerna
        addr <= X"FFFC";
        wait for CLK_PERIOD;
        assert do = X"00" report "[ERROR] Bad Reset vector LSB!" severity error;

        addr <= X"FFFD";
        wait for CLK_PERIOD;
        assert do = X"C0" report "[ERROR] Bad Reset vector MSB!" severity error;

        -----------------------------------------------------------------------
        -- AVSLUT
        -----------------------------------------------------------------------
        report "[TB] ALL TESTS PASSED SUCCESSFULLY! THE INTERFACE IS ROCK SOLID.";
        sim_done <= true;
        wait;
    end process;

end sim;
