library ieee;
use ieee.std_logic_1164.all;
  
entity my_bb_vhdl is
  port(
    t_clk : in std_logic;
    t_data_in : in std_logic_vector(7 downto 0);
    t_data_out : out std_logic_vector(7 downto 0)
  );

end entity;

architecture sim of my_bb_vhdl is

  -- references the verilog black box (bb.v)
  component my_black_box
    port(
      clk : in std_logic;
      data_in : in std_logic_vector(7 downto 0);
      data_out : out std_logic_vector(7 downto 0)
    );
  end component;

  signal s_data : std_logic_vector(7 downto 0);

begin

  thebb : my_black_box
    port map (
      clk => t_clk,
      data_in => t_data_in,
      data_out => t_data_out
      );
  
end architecture;
