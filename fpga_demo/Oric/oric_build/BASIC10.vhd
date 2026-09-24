library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity BASIC10 is
port (
	clk  : in  std_logic;
	addr : in  std_logic_vector(13 downto 0);
	data : out std_logic_vector(7 downto 0)
);
end entity;

architecture Behavioral of BASIC10 is
begin
    process(addr)
    begin
        case to_integer(unsigned(addr)) is
            -- =================================================================
            -- 6502 RESET-VEKTOR (Sista två bytesen i 14-bitarsrymden)
            -- =================================================================
            when 16380 => data <= std_logic_vector(to_unsigned(16#00#, 8)); -- Reset LSB ($00)
            when 16381 => data <= std_logic_vector(to_unsigned(16#C0#, 8)); -- Reset MSB ($C0) -> Startar på $C000

            -- =================================================================
            -- AVANCERAT STRIDS-TESTPROGRAM (Verifierar Stacken, JSR, RTS och RAM)
            -- =================================================================
            -- Huvudprogram (Startar vid $C000 / Offset 0)
            when 0  => data <= std_logic_vector(to_unsigned(16#A9#, 8)); -- [C000] LDA #$A5 (Kryptonit-mönster)
            when 1  => data <= std_logic_vector(to_unsigned(16#A5#, 8)); 
            
            when 2  => data <= std_logic_vector(to_unsigned(16#48#, 8)); -- [C002] PHA (Tryck ner mönstret på stacken!)
            
            when 3  => data <= std_logic_vector(to_unsigned(16#A9#, 8)); -- [C003] LDA #$00 (Nollställ A)
            when 4  => data <= std_logic_vector(to_unsigned(16#00#, 8)); 
            
            when 5  => data <= std_logic_vector(to_unsigned(16#20#, 8)); -- [C005] JSR $C020 (Hoppa till subrutin längre fram!)
            when 6  => data <= std_logic_vector(to_unsigned(16#20#, 8)); -- Subrutin LSB ($20)
            when 7  => data <= std_logic_vector(to_unsigned(16#C0#, 8)); -- Subrutin MSB ($C0)
            
            -- Hit återvänder vi efter subrutinen (RTS hämtar adressen från stacken)
            when 8  => data <= std_logic_vector(to_unsigned(16#68#, 8)); -- [C008] PLA (Hämta tillbaka mönstret från stacken till A)
            
            when 9  => data <= std_logic_vector(to_unsigned(16#C9#, 8)); -- [C009] CMP #$A5 (Stämmer allt?)
            when 10 => data <= std_logic_vector(to_unsigned(16#A5#, 8)); 
            
            when 11 => data <= std_logic_vector(to_unsigned(16#F0#, 8)); -- [C00B] BEQ SUCCESS
            when 12 => data <= std_logic_vector(to_unsigned(16#01#, 8)); -- Hoppa +1 byte
            
            when 13 => data <= std_logic_vector(to_unsigned(16#4C#, 8)); -- [C00D] JMP $C00D (FAIL-TRAP)

            -- SUCCESS-TRAP ($C00F / Offset 15)
            when 15 => data <= std_logic_vector(to_unsigned(16#4C#, 8)); -- [C00F] JMP $C00F
            when 16 => data <= std_logic_vector(to_unsigned(16#0F#, 8)); -- LSB ($0F)
            when 17 => data <= std_logic_vector(to_unsigned(16#C0#, 8)); -- MSB ($C0)

            -- =================================================================
            -- SUBRUTIN (Mappad till Offset 32 / Systemadress $C020)
            -- =================================================================
            when 32 => data <= std_logic_vector(to_unsigned(16#8D#, 8)); -- [C020] STA $1234 (Skriv det nollställda A till RAM)
            when 33 => data <= std_logic_vector(to_unsigned(16#34#, 8)); -- LSB
            when 34 => data <= std_logic_vector(to_unsigned(16#12#, 8)); -- MSB
            
            when 35 => data <= std_logic_vector(to_unsigned(16#60#, 8)); -- [C023] RTS (Gå tillbaka! Testar om stack-RAM fungerar)

            -- Skottsäker guard för resten av ROM-minnet
            when others => data <= std_logic_vector(to_unsigned(16#EA#, 8)); -- NOP
        end case;
    end process;
end architecture;
