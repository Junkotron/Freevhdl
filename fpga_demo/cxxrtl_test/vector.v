module myvector (
		 input [3:0] vec,
		 output	     y
);
   // Bitvis AND 
   assign y = (vec[0] & vec[1] & vec[2] & vec[3]); 
endmodule

