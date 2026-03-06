module CPLD (
    // 1. Clock Input
    input wire clk_8khz,  // Pin 33

    // 2. Button Inputs (Active Low)
    input wire btn1,      // Pin 1 (-1 Decrement)
    input wire btn2,      // Pin 2 (+1 Increment)
    input wire btn4,      // Pin 4 (+1 Increment)

    // 3. MCU Outputs (Reporting lower 5 bits of counter)
    output wire mcu_28,   // Pin 28 (Bit 0)
    output wire mcu_29,   // Pin 29 (Bit 1)
    output wire mcu_31,   // Pin 31 (Bit 2)
    output wire mcu_34,   // Pin 34 (Bit 3)
    output wire mcu_37,   // Pin 37 (Bit 4)

    // 4. 7-Segment Segments (Active Low for Common Anode)
    output reg seg_a,     // Pin 5
    output reg seg_b,     // Pin 6
    output reg seg_c,     // Pin 8
    output reg seg_d,     // Pin 9
    output reg seg_e,     // Pin 11
    output reg seg_f,     // Pin 12
    output reg seg_g,     // Pin 14
    output reg seg_dp,    // Pin 16

    // 5. 7-Segment Common Anodes (Active High)
    output reg com1,      // Pin 18 (Digit 4)
    output reg com2,      // Pin 19 (Digit 3)
    output reg com3,      // Pin 20 (Digit 2 - Tens)
    output reg com4       // Pin 21 (Digit 1 - Units)
);

    // =========================================================
    // [1] Button Debouncing & Direction Logic
    // =========================================================
    wire btn_up = ~btn2 | ~btn4; // 버튼 2나 4가 눌리면 증가
    wire btn_dn = ~btn1;         // 버튼 1이 눌리면 감소
    wire any_btn = btn_up | btn_dn; // 아무 버튼이나 눌렸는지 확인

    reg [7:0] debounce_cnt = 8'd0;
    reg btn_state = 1'b0;
    reg btn_prev = 1'b0;

    always @(posedge clk_8khz) begin
        if (any_btn == btn_state) begin
            debounce_cnt <= 8'd0;
        end else begin
            debounce_cnt <= debounce_cnt + 1;
            if (debounce_cnt == 8'd150) begin 
                btn_state <= any_btn;
                debounce_cnt <= 8'd0;
            end
        end
        btn_prev <= btn_state;
    end

    // 버튼이 막 눌린 순간(Rising Edge)
    wire btn_trigger = (btn_state == 1'b1 && btn_prev == 1'b0);

    // =========================================================
    // [2] 5-bit Counter (0 ~ 31)
    // =========================================================
    // 0~31은 총 32개의 상태이므로 5비트로 완벽하게 표현 가능
    reg [4:0] count = 5'd0;

    always @(posedge clk_8khz) begin
        if (btn_trigger) begin
            if (btn_dn) begin
                // 감소 로직 (-1)
                if (count == 5'd0) count <= 5'd31; // 0에서 내리면 31로 순환
                else count <= count - 1;
            end else if (btn_up) begin
                // 증가 로직 (+1)
                if (count == 5'd31) count <= 5'd0; // 31에서 올리면 0으로 순환
                else count <= count + 1;
            end
        end
    end

    // 카운터 5비트를 MCU의 5개 핀으로 1:1 매핑하여 실시간 전송
    assign mcu_28 = count[0];
    assign mcu_29 = count[1];
    assign mcu_31 = count[2];
    assign mcu_34 = count[3];
    assign mcu_37 = count[4];

    // =========================================================
    // [3] Ultra-light BCD Converter (for 0~31 only)
    // =========================================================
    reg [3:0] tens, units;
    
    // 최대값이 31이므로, 뺄셈기 몇 개로 10의 자리와 1의 자리를 완벽 분리 가능
    always @(count) begin
        if (count >= 30) begin 
            tens = 4'd3; units = count - 5'd30; 
        end else if (count >= 20) begin 
            tens = 4'd2; units = count - 5'd20; 
        end else if (count >= 10) begin 
            tens = 4'd1; units = count - 5'd10; 
        end else begin 
            tens = 4'd0; units = count; 
        end
    end

    // =========================================================
    // [4] FND Dynamic Multiplexing
    // =========================================================
    reg [4:0] mux_div = 5'd0;
    always @(posedge clk_8khz) begin
        mux_div <= mux_div + 1;
    end

    wire [1:0] digit_sel = mux_div[4:3];
    reg [3:0] current_val;

    always @(*) begin
        com1 = 1'b0; com2 = 1'b0; com3 = 1'b0; com4 = 1'b0;
        
        case (digit_sel)
            2'b00: begin com1 = 1'b1; current_val = 4'hF; end // 1000의 자리 (끄기)
            2'b01: begin com2 = 1'b1; current_val = 4'hF; end // 100의 자리 (끄기)
            2'b10: begin com3 = 1'b1; current_val = tens; end // 10의 자리 출력
            2'b11: begin com4 = 1'b1; current_val = units; end // 1의 자리 출력
        endcase
    end

    // =========================================================
    // [5] 7-Segment Decoder
    // =========================================================
    always @(*) begin
        case (current_val)
            4'h0: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0000001;
            4'h1: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b1001111;
            4'h2: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0010010;
            4'h3: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0000110;
            4'h4: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b1001100;
            4'h5: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0100100;
            4'h6: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0100000;
            4'h7: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0001111;
            4'h8: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0000000;
            4'h9: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b0000100;
            default: {seg_a, seg_b, seg_c, seg_d, seg_e, seg_f, seg_g} = 7'b1111111; // F 입력 시 끄기
        endcase
        seg_dp = 1'b1; 
    end

endmodule