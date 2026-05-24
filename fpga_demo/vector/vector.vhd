library ieee;
use ieee.std_logic_1164.all;
  
entity myvector is
  port(
    vec : in std_logic_vector(3 downto 0);
    y   : out std_logic;
    yinv : out std_logic
  );

end entity;
  
architecture sim of myvector is

  signal s_yinv : std_logic;
  
begin

  s_yinv <= (vec(0) and vec(1) and vec(2) and vec(3));
  y <= not s_yinv;
  
  
end architecture;
