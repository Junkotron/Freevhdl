library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity top_t65_system_sim is
end entity top_t65_system_sim;
  
architecture sim of top_t65_system_sim is

  signal husklocka: std_logic;
begin
  computer: entity work.top_t65_system(rtl)
    generic map(
      CLK_FREQ_HZ => 20, -- short reset with simulation
      DELAY => X"02"         -- Bareable in wave viewer...
      )
    port map(
      clk_in => husklocka
      );

  -- should give us a nice 100 MHz square
  ticktack: process
  begin
    wait for 5 ns;
    husklocka <= '0';
    wait for 5 ns;
    husklocka <= '1';
  end process;
            

end architecture sim;
