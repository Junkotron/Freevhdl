(* top *)
module top (
    input wire clk,
    input wire [7:0] io_in,
    output wire [7:0] io_out
);
    // Instantiate it cleanly
    my_black_box bb_inst (
        .clk(clk),
        .data_in(io_in),
        .data_out(io_out)
    );
endmodule
