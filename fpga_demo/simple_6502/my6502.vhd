library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity top_t65_system is
  generic (
    CLK_FREQ_HZ : INTEGER := 100_000_000
  );
  port (
    clk_in   : in  std_logic;                     -- Systemklocka (t.ex. 10 MHz)
    led_out  : out std_logic_vector(7 downto 0);   -- Exempelutgång för att se att CPU:n kör
    CLK_1MHz : out std_logic;

    led1 : out std_logic;
    led2 : out std_logic;
  
    atest : out std_logic
  );
end entity top_t65_system;

architecture rtl of top_t65_system is

  -- 1. Deklarera T65-komponenten (Standard Open-Core T65)
  -- removed
  
  -- 2. Deklarera vår latch-fria startup-reset
  --
  
  -- Interna signaler för att binda ihop CPU, Reset och Minne
  signal s_reset_n     : std_logic;
  signal cpu_rw_n    : std_logic;
  signal cpu_addr    : std_logic_vector(23 downto 0);
  signal cpu_data_in : std_logic_vector(7 downto 0);
  signal cpu_data_out: std_logic_vector(7 downto 0);

  signal addrlo : std_logic_vector(7 downto 0);
  signal addrhi : std_logic_vector(7 downto 0);
  
  -- Ett enkelt internt minne (ROM) fyllt med 6502-maskinkod
  type rom_type is array (0 to 31) of std_logic_vector(7 downto 0);
  -- Programmet gör: LDA #$55 -> STA $4000 -> JMP $F000 (Loopar oändligt)
  constant my_rom : rom_type := (
    0 => X"A9", 1 => "01010011",        -- LDA #$53(Ladda ackumulatorn med hex 53)
    2 => X"8D", 3 => X"00", 4 => X"40", -- STA $4000 (Skriv till LED-adressen)
    5 => X"EA", 6 => X"EA", 7 => X"EA",  -- NOPs to make 50% duty approx
    8 => X"A9", 9 => "01010010",        -- LDA #$52 (Ladda ackumulatorn med hex 52)
    10 => X"8D", 11 => X"00", 12 => X"40", -- STA $4000 (Skriv till LED-adressen)
    13 => X"4C", 14 => X"00", 15 => X"F0", -- $F005: JMP $F000 (Hoppa tillbaka till start)

    -- Fyll resten med NOP (No Operation) fram till reset-vektorerna
    16 => X"EA", 17 => X"EA", 18 => X"EA", 19 => X"EA",
    20 => X"EA", 21 => X"EA", 22 => X"EA", 23 => X"EA",
    24 => X"EA", 25 => X"EA", 26 => X"EA", 27 => X"EA",
    
    -- $FFFC och $FFFD: Reset Vector (Pekar på var CPU ska starta: $F000)
    28 => X"00",             -- $FFFC: Low byte av startadress
    29 => X"F0",              -- $FFFD: High byte av startadress

    30 => X"EA", 31 => X"EA"
    );

  signal reg_leds : std_logic_vector(7 downto 0) := X"00";

  signal cpuclk : std_logic;

begin

  addrlo <= cpu_addr(7 downto 0);
  addrhi <= cpu_addr(15 downto 8);
  
--  atest <= cpu_addr(1);
--  atest <= cpu_data_in(1);
  atest <= reg_leds(0);
  led1 <= reg_leds(0);
  led2 <= reg_leds(1);
  
  
  -- obtain 25MHz for 6502... the important thing is that the memory
  -- handler is quicker
  div1mhz: entity work.clock_divider_n(Behavioral)
    generic map (
      N => 100
    )
    port map (
      clk_in => clk_in,
      reset => '0',
      clk_out => cpuclk
    );

  CLK_1MHz <= cpuclk;

  
  -- Koppla utgången till vårt interna LED-register
  led_out <= reg_leds;

  -- Instansiera återställningsmodulen (1 sekund vid 100 MHz)
  u_reset : entity work.startup_reset
    generic map (
      CLK_FREQ_HZ => CLK_FREQ_HZ
      )
    port map (
      clk       => clk_in,
      reset_out => s_reset_n -- Detta ger en ren '0' i en sekund, sedan '1'
      );

  inst_cpu2 : ENTITY work.T65(rtl)
    PORT MAP(
      Mode => "00",
      Res_n => s_reset_n,       -- Styrs av vår säkra startup-reset
      Enable => '1',
      Clk => cpuclk,
      Rdy => '1',
      Abort_n => '1',
      IRQ_n => '1',
      NMI_n => '1',
      SO_n => '1',
      R_W_n => cpu_rw_n,
      A => cpu_addr,
      DI => cpu_data_in,
      DO => cpu_data_out
      );

  -- Synkron minneshantering och I/O (Helt fritt från dolda latchar)
  process(clk_in)
    variable rom_index : integer;
  begin
    if rising_edge(clk_in) then
      if s_reset_n = '0' then
        cpu_data_in <= X"00";
        reg_leds    <= X"00";
      else
        -- Standardvärde på databuss in till CPU för att undvika flytande tillstånd
        cpu_data_in <= X"EA"; -- NOP instruktion som fallback

        -- 1. Avkoda Reset Vector ($FFFC - $FFFD) samt ROM ($F000 - $F007)
        if cpu_addr(15 downto 4) = X"FFF" or cpu_addr(15 downto 4) = "111100000000" then
          -- Räkna ut index i vår lilla 32-bytes array
          rom_index := to_integer(unsigned(cpu_addr(4 downto 0)));
          cpu_data_in <= my_rom(rom_index);
          
        -- 2. Avkoda I/O-port för lysdioder (Skrivning till adress $4000)
        elsif cpu_addr(15 downto 0) = X"4000" then
          if cpu_rw_n = '0' then -- CPU skriver
            reg_leds <= cpu_data_out;
          end if;
        end if;
      end if;
    end if;
  end process;

end architecture rtl;
