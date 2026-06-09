library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity top_t65_system_gowin is
  port (
    husklocka: in std_logic;
    atest : out std_logic;

    -- Avoid indices in .cst file like the plagues
    led1, led2, led3, led4, led5, led6, led7, led8 : out std_logic
    );
end entity top_t65_system_gowin;
  
architecture rtl of top_t65_system_gowin is
  signal s_atest : std_logic;

  signal s_led_out  : std_logic_vector(7 downto 0);

  
  
begin
  computer: entity work.top_t65_system(rtl)
    port map(
      clk_in => husklocka,
      led_out => s_led_out,
      atest => s_atest
      );

  atest <= s_atest;

  led1 <= s_led_out(0);
  led2 <= s_led_out(1);
  led3 <= s_led_out(2);
  led4 <= s_led_out(3);
  led5 <= s_led_out(4);
  led6 <= s_led_out(5);
  led7 <= s_led_out(6);
  led8 <= s_led_out(7);
  
end architecture rtl;
