module led(
    input clk,
    output [7:0] leds // Number depends on your board's LED count (e.g., 6 for 9K)
);

reg [32:0] counter;
reg [7:0] led_state;

// 27 MHz clock divider
always @(posedge clk) begin
    counter <= counter + 1'b1;
    if (counter == 27_000_000) begin // .1 seconds
        counter <= 0;
        led_state <= led_state + 1'b1;
    end
end

   assign leds = led_state;
   
   

endmodule
