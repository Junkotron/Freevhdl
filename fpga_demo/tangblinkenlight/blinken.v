module led #(parameter N_LEDS=8,
	     parameter LED_ON_WHEN=1'b1,
	     parameter DIVN=100_000_000
	     )
(
    
 
 input		     clk,
 output [N_LEDS-1:0] leds // Number depends on your board's LED count (e.g., 6 for 9K)
 );
   
   reg [32:0] counter;
   reg [N_LEDS-1:0] led_state;
   wire [N_LEDS-1:0] led_driver;
   
   // NDIV clock divider
   always @(posedge clk) begin
      counter <= counter + 1'b1;
      if (counter == DIVN) begin // 1 seconds?
         counter <= 0;
	 led_state <= led_state + 1'b1;
	 if (LED_ON_WHEN == 1) begin
	    led_driver <= led_state;
	 end
	 else begin
	    // Invert leds so we make a zero for the on leds...
	    led_driver <= {N_LEDS{1'b1}} ^led_state;
	 end
      end
    end


/*
   assign leds[0] = 1 ^ led_state[0];
   assign leds[1] = 1 ^ led_state[1];
   assign leds[2] = (LEDS_ON_WHEN) ^ led_state[2];
   assign leds[3] = LEDS_ON_WHEN ^ led_state[3];
   assign leds[4] = LEDS_ON_WHEN ^ led_state[4];
   assign leds[5] = LEDS_ON_WHEN ^ led_state[5];
  */ 
   
//   assign leds = {N_LEDS{LEDS_ON_WHEN}}^led_state;
   assign leds = led_driver;
   
   

endmodule
