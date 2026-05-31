

module divider25000  (
    input  logic clk,
    output logic clk_out
);
   logic [15:0] pos_count;
   
   logic	clk_p;
   

   wire logprobe_bit;
   localparam bit [15:0] DIVIDE_BY = 32'd50000;

   assign clk_out = clk_p;
   assign logprobe_bit = pos_count[8];
   
    // Generate output on positive edge
    always_ff @(posedge clk) begin
       if (pos_count >= (DIVIDE_BY - 1)) begin
            pos_count <= '0;
            clk_p     <= ~clk_p;
        end else begin
            pos_count <= pos_count + 1'b1;
        end
    end

   
endmodule
