
module sttop 
  (
   input logic	clk_in,	  // Main onboard oscillator (e.g., 100MHz)
   output logic	clk_1kHz // signal to logic analyzer
   );
   
   
		       
    // 2. Instantiate the generic clock divider
    divider25000  u_clk_divider 
      (
       .clk(clk_in),           // Connect to system clock
       .clk_out(clk_1kHz)      // Output slow clock signal
       );

endmodule
