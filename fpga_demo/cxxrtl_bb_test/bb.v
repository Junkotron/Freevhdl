(* cxxrtl_blackbox *)
module my_black_box (
    input wire clk,
    input wire [7:0] data_in,
    output reg [7:0] data_out // Changed wire to reg
);
    // Explicit register update logic forces Yosys to flag this 
    // module as a critical, un-removable behavioral simulation block
    always @(posedge clk) begin
        data_out <= data_in;
    end
endmodule
