module Switches_To_LEDs
 (input switch_1,
  input switch_2,
  input switch_3,
  input switch_4,
  output led_1,
  output led_2,
  output led_3,
  output led_4);
 
  assign led_1 = switch_1;
  assign led_2 = switch_2;
  assign led_3 = switch_3;
  assign led_4 = switch_4;
endmodule