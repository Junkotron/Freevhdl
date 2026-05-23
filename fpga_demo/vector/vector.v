module myvector (
		 input [3:0] vec,
		 output	     y
);
    // Bitvis AND följt av bitvis NOT
    assign y = ~(vec[0] & vec[1]); 
endmodule

