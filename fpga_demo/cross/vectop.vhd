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

  component myvector
    port(
      ein : in std_logic;
      vec : in std_logic_vector(3 downto 0);
      y   : out std_logic;
      eout : out std_logic
      );
  end component;
  
begin

  thevec : myvector
    port map(
      ein => '0',
      vec => vec,
      y => y,
      eout => dum
      );
  
end architecture;
