library ieee;
use ieee.std_logic_1164.all;
  
entity myvectop is
  port(
    vec : in std_logic_vector(3 downto 0);
    y   : out std_logic
  );

end entity;
  
architecture sim of myvectop is

  signal dum : std_logic;
  
begin

  thevec : entity work.myvector(sim)
    port map(
      ein => '0',
      vec => vec,
      y => y,
      eout => dum
      );
  
end architecture;
