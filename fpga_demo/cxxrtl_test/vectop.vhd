library ieee;
use ieee.std_logic_1164.all;
  
entity myvectop is
  port(
    vec : in std_logic_vector(3 downto 0);
    y   : out std_logic
  );

end entity;

architecture sim of myvectop is

  component myvector
    port(
      vec : in std_logic_vector(3 downto 0);
      y   : out std_logic
      );
  end component;

  signal noty : std_logic;
  
begin

  y <= not noty;
  
  thevec : myvector
    port map(
      vec => vec,
      y => noty
      );
  
end architecture;
