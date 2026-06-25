import os

# Sökvägar till dina filer
bin_path = "oric_test.rom"
vhdl_path = "BASIC10.vhd"

if not os.path.exists(bin_path):
    print(f"Fel: Hittade inte källfilen {bin_path}!")
    exit(1)

with open(bin_path, "rb") as f:
    rom_bytes = f.read()

# Verifiera storleken så att allt stämmer
print(f"Läser {bin_path} ({len(rom_bytes)} bytes)...")

vhdl_content = """-- Genererad automatisk ROM-fil för Oric FPGA
library ieee;
use ieee.std_logic_1164.all,ieee.numeric_std.all;

entity BASIC10 is
port (
	clk  : in  std_logic;
	addr : in  std_logic_vector(13 downto 0);
	data : out std_logic_vector(7 downto 0)
);
end entity;

architecture prom of BASIC10 is
	type rom is array(0 to  16383) of std_logic_vector(7 downto 0);
	signal rom_data: rom := (
"""

# Formatera byten i block om 8 per rad för maximal skannbarhet
lines = []
for i in range(0, len(rom_bytes), 8):
    chunk = rom_bytes[i:i+8]
    hex_str = ", ".join(f'X"{b:02X}"' for b in chunk)
    lines.append(f"        {hex_str}")

# Sätt ihop allt och avsluta VHDL-strukturen
vhdl_content += ",\n".join(lines)
vhdl_content += """
begin
process(clk)
begin
	if rising_edge(clk) then
		data <= rom_data(to_integer(unsigned(addr)));
	end if;
end process;
end architecture;
"""

with open(vhdl_path, "w") as f:
    f.write(vhdl_content)

print(f"Klart! Sparade {vhdl_path}")

