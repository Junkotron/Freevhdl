library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity top_t65_system_gowin is
  port (
    husklocka: in std_logic
    );
end entity top_t65_system_gowin;
  
architecture rtl of top_t65_system_gowin is

  
begin
  computer: entity work.top_t65_system(rtl)
    port map(
      clk_in => husklocka
      );
            

end architecture rtl;
