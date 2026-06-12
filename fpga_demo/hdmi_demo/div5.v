module clk_div_5 (
    input  wire clk_in,
    input  wire rst_n,
    output wire clk_out
);

    reg [2:0] pos_count;
    reg [2:0] neg_count;

    // 1. Räknare som triggas på positiv flank (posedge)
    always @(posedge clk_in or negedge rst_n) begin
        if (!rst_n) begin
            pos_count <= 3'd0;
        end else if (pos_count == 3'd4) begin
            pos_count <= 3'd0;
        end else begin
            pos_count <= pos_count + 1'b1;
        end
    end

    // 2. Räknare som triggas på negativ flank (negedge)
    always @(negedge clk_in or negedge rst_n) begin
        if (!rst_n) begin
            neg_count <= 3'd0;
        end else if (neg_count == 3'd4) begin
            neg_count <= 3'd0;
        end else begin
            neg_count <= neg_count + 1'b1;
        end
    end

    // 3. Skapa klocksignaler som är höga i 2 av 5 cykler
    // Genom att göra en OR-operation på dessa fasförskjutna signaler får vi exakt 50% duty cycle.
    wire clk_pos = (pos_count < 3'd2);
    wire clk_neg = (neg_count < 3'd2);

    assign clk_out = clk_pos | clk_neg;

endmodule
