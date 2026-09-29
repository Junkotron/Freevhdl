library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity m6522_tb is
end entity;

architecture sim of m6522_tb is
    signal clk         : std_logic := '0';
    signal reset_l     : std_logic := '0';
    signal ena_4       : std_logic := '1';
    signal i_rs        : std_logic_vector(3 downto 0) := (others => '0');
    signal i_data      : std_logic_vector(7 downto 0) := (others => '0');
    signal o_data      : std_logic_vector(7 downto 0);
    signal o_data_oe_l : std_logic;
    signal i_rw_l      : std_logic := '1';
    signal i_cs1       : std_logic := '0';
    signal i_cs2_l     : std_logic := '1';
    signal o_irq_l     : std_logic;
    signal i_p2_h      : std_logic := '1';
    
    signal dummy_in    : std_logic_vector(7 downto 0) := (others => '0');
    signal dummy_out   : std_logic_vector(7 downto 0);
    signal dummy_oe    : std_logic_vector(7 downto 0);
    signal dummy_logic : std_logic := '0';
begin

    dut: entity work.M6522
    port map (
        I_RS         => i_rs,
        I_DATA       => i_data,
        O_DATA       => o_data,
        O_DATA_OE_L  => o_data_oe_l,
        I_RW_L       => i_rw_l,
        I_CS1        => i_cs1,
        I_CS2_L      => i_cs2_l,
        O_IRQ_L      => o_irq_l,
        I_CA1        => dummy_logic,
        I_CA2        => dummy_logic,
        O_CA2        => open,
        O_CA2_OE_L   => open,
        I_PA         => dummy_in,
        O_PA         => dummy_out,
        O_PA_OE_L    => dummy_oe,
        I_CB1        => dummy_logic,
        O_CB1        => open,
        O_CB1_OE_L   => open,
        I_CB2        => dummy_logic,
        O_CB2        => open,
        O_CB2_OE_L   => open,
        I_PB         => dummy_in,
        O_PB         => dummy_out,
        O_PB_OE_L    => dummy_oe,
        I_P2_H       => i_p2_h,
        RESET_L      => reset_l,
        ENA_4        => ena_4,
        CLK          => clk
    );

    clk_process : process
    begin
        clk <= '0';
        wait for 5 ns;
        clk <= '1';
        wait for 5 ns;
    end process;

    stim_process : process
        procedure cpu_write(addr : std_logic_vector(3 downto 0); data : std_logic_vector(7 downto 0)) is
        begin
            wait until rising_edge(clk);
            i_rs    <= addr;
            i_data  <= data;
            i_cs1   <= '1';
            i_cs2_l <= '0';
            i_rw_l  <= '0';
            i_p2_h  <= '1';
            wait until rising_edge(clk);
            i_cs1   <= '0';
            i_cs2_l <= '1';
            i_rw_l  <= '1';
        end procedure;
    begin
        reset_l <= '0';
        wait for 40 ns;
        reset_l <= '1';
        wait for 40 ns;

        report "--- Start VIA Timer 1 VHDL test ---";

        cpu_write("1110", x"C0");
        cpu_write("1011", x"40");
        cpu_write("0100", x"05");
        cpu_write("0101", x"00");

        wait for 1 us;

        if o_irq_l = '0' then
            report "SUCCESS: IRQ activated by Timer 1!" severity note;
        else
            report "ERROR: Timer failed to pull IRQ low!" severity error;
        end if;

        wait;
    end process;

end architecture;
