module myvector (
		 input ein,
		 input [3:0] vec,
		 output	     y,
		 output eout
);
   assign eout = ~ein;

   // Bitvis AND följt av bitvis NOT
   assign y = ~(vec[0] & vec[1]); 
endmodule

