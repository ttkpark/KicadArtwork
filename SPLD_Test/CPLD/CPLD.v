module CPLD (
    input wire test_in,  // Pin Planner에서 28번 핀으로 할당할 포트
    output wire test_out  // Pin Planner에서 28번 핀으로 할당할 포트
);

    // 28번 핀을 항상 High(1) 상태로 출력
    assign test_out = !test_in;

endmodule