module LED_Toggle
 (input i_clk,
  input i_switch_1,
  input i_switch_2,
  input i_switch_3,
  input i_switch_4,
  output o_led_1,
  output o_led_2,
  output o_led_3,
  output o_led_4);
 
  reg r_led_1 = 1'b0;
  reg r_switch_1 = 1'b0;

  reg r_led_3 = 1'b0;
  reg r_switch_3 = 1'b0;

  always @(posedge i_clk)
  begin
    r_switch_1 <= i_switch_1;
    if (i_switch_1 == 1'b0 && r_switch_1 == 1'b1)
    begin
      r_led_1 <= ~r_led_1;
    end

    r_switch_3 <= i_switch_3;
    if (i_switch_3 == 1'b0 && r_switch_3 == 1'b1)
    begin
      r_led_3 <= ~r_led_3;
    end
  end
  
  assign o_led_1 = r_led_1;
  assign o_led_2 = i_switch_2 && i_switch_1;
  assign o_led_3 = r_led_3;
  assign o_led_4 = i_switch_4 && r_led_3;
endmodule