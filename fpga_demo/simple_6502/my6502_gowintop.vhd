library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity top_t65_system_gowin is
  port (
    husklocka: in std_logic;
    led1 : out std_logic;
    led2 : out std_logic;
    atest : out std_logic
    );
end entity top_t65_system_gowin;
  
architecture rtl of top_t65_system_gowin is
  signal s_atest : std_logic;
  
begin
  computer: entity work.top_t65_system(rtl)
    port map(
      clk_in => husklocka,
      led1 => led1,
      led2 => led2,
      atest => s_atest
      );

  atest <= s_atest;

end architecture rtl;
