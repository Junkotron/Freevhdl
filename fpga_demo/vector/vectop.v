

module myvectop (
		 input [3:0] vec,
		 output	     y
);

   wire dum;
   
   myvector thevec(0, vec, y, dum);
   

endmodule

