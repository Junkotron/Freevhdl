-- Import the standard library for logic types
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Define the inputs and outputs of the NAND gate
entity nand_gate is
    Port ( 
        A : in  STD_LOGIC;  -- Input 1
        B : in  STD_LOGIC;  -- Input 2
        Y : out STD_LOGIC   -- Output
    );
end nand_gate;

-- Define the internal behavior of the NAND gate
architecture Dataflow of nand_gate is
begin
    -- Concurrent signal assignment using the built-in 'nand' operator
    Y <= A nand B;
end Dataflow;

