LIBRARY ieee;
USE ieee.std_logic_1164.all;

--LIBRARY altera_mf;
--USE altera_mf.altera_mf_components.all;

entity dpram is
	generic (
		 addr_width_g : integer := 8;
		 data_width_g : integer := 8
	);
	PORT
	(
		address_a	: IN STD_LOGIC_VECTOR (addr_width_g-1 DOWNTO 0);
		address_b	: IN STD_LOGIC_VECTOR (addr_width_g-1 DOWNTO 0) := (others => '0');
		clock_a		: IN STD_LOGIC  := '1';
		clock_b		: IN STD_LOGIC  := '1';
		data_a		: IN STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0);
		data_b		: IN STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0) := (others => '0');
		enable_a		: IN STD_LOGIC  := '1';
		enable_b		: IN STD_LOGIC  := '1';
		wren_a		: IN STD_LOGIC  := '0';
		wren_b		: IN STD_LOGIC  := '0';
		q_a			: OUT STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0);
		q_b			: OUT STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0)
	);
END dpram;


ARCHITECTURE SYN OF dpram IS
BEGIN
END SYN;
