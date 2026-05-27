
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity clock_divider_n is
    Generic (
        N : integer := 10 -- Change this generic to divide by N
    );
    Port (
        clk_in  : in  std_logic;
        reset   : in  std_logic;
        clk_out : out std_logic
    );
end clock_divider_n;

architecture Behavioral of clock_divider_n is
    -- Calculate required bit-width dynamically for the counter
    subtype counter_t is integer range 0 to N - 1;
    signal count : counter_t := 0;
    signal tmp_clk : std_logic := '0';
begin

    process(clk_in, reset)
    begin
        if reset = '1' then
            count <= 0;
            tmp_clk <= '0';
        elsif rising_edge(clk_in) then
            -- Toggle clock when counter reaches N/2 - 1
            if count = (N / 2) - 1 then
                tmp_clk <= not tmp_clk;
                count <= count + 1;
            elsif count = N - 1 then
                tmp_clk <= not tmp_clk;
                count <= 0;
            else
                count <= count + 1;
            end if;
        end if;
    end process;

    clk_out <= tmp_clk;

end Behavioral;
