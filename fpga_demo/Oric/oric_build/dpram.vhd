LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY dpram IS
	GENERIC (
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
		enable_a	: IN STD_LOGIC  := '1';
		enable_b	: IN STD_LOGIC  := '1';
		wren_a		: IN STD_LOGIC  := '0';
		wren_b		: IN STD_LOGIC  := '0';
		q_a			: OUT STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0);
		q_b			: OUT STD_LOGIC_VECTOR (data_width_g-1 DOWNTO 0)
	);
END dpram;

ARCHITECTURE behavior OF dpram IS
	type mem_type is array (0 to (2**addr_width_g) - 1) of std_logic_vector(data_width_g-1 downto 0);
	signal shared_memory : mem_type := (others => (others => '0'));
BEGIN

	-- Port A
	process(clock_a)
	begin
		if rising_edge(clock_a) then
			if enable_a = '1' then
				if wren_a = '1' then
					shared_memory(to_integer(unsigned(address_a))) <= data_a;
				end if;
				q_a <= shared_memory(to_integer(unsigned(address_a)));
			end if;
		end if;
	end process;

	-- Port B
	process(clock_b)
	begin
		if rising_edge(clock_b) then
			if enable_b = '1' then
				if wren_b = '1' then
					shared_memory(to_integer(unsigned(address_b))) <= data_b;
				end if;
				q_b <= shared_memory(to_integer(unsigned(address_b)));
			end if;
		end if;
	end process;

END behavior;
