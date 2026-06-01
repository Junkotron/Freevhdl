
-- Courtesy of googles ai :)

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity startup_reset is
    generic (
        CLK_FREQ_HZ : integer := 25_000_000 -- Adjust to match your board clock
    );
    port (
        clk       : in  std_logic;
        reset_out : out std_logic
    );
end entity startup_reset;

architecture rtl of startup_reset is
    -- Counter to hold the 1-second delay
    signal rst_counter : integer range 0 to CLK_FREQ_HZ := 0;
    
    -- Internal register initialized to '0' (Active-Low Reset active on startup)
    signal rst_reg     : std_logic := '0';
begin

    -- Drive output directly from the register
    reset_out <= rst_reg;

    -- Synchronous Counter Process
    process(clk)
    begin
        if rising_edge(clk) then
            if rst_counter < CLK_FREQ_HZ then
                rst_counter <= rst_counter + 1;
                rst_reg     <= '0'; -- Pull low during counting
            else
                rst_reg     <= '1'; -- Release reset after 1 second
                -- rst_counter stays at maximum value, preventing rollover
            end if;
        end if;
    end process;

end architecture rtl;
