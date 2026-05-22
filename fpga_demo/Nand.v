module nand_gate (
    input  a,
    input  b,
    output y
);
    // Bitvis AND följt av bitvis NOT
    assign y = ~(a & b); 
endmodule

