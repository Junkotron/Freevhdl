
-- Courtesy of googles ai :)
library ieee;
use ieee.std_logic_1164.all;

entity startup_reset is
    port (
        clk        : in  std_logic;
        global_rst : out std_logic  -- Active high reset for your main logic
    );
end startup_reset;

architecture rtl of startup_reset is
    -- Initialize a 4-bit shift register strictly to '0's.
    -- FPGAs naturally back this initialization value into hardware registers.
    signal rst_shift : std_logic_vector(3 downto 0) := "0000";
begin

    process(clk)
    begin
        if rising_edge(clk) then
            -- On every clock cycle, shift in a '1' from the right.
            -- This creates a pipeline of zeros that slowly vanishes.
            rst_shift <= rst_shift(2 downto 0) & '1';
        end if;
    end process;

    -- The reset signal is driven by the final bit of the shift register.
    -- While the '1' ripples through the pipeline, global_rst remains active ('1').
    -- Once the pipeline is completely full of '1's, global_rst drops to '0' permanently.
    global_rst <= not rst_shift(3);

end rtl;
