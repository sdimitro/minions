library ieee;
use ieee.std_logic_1164.all;

entity Switches_To_LEDs is
  port (
    switches_1 : in std_logic;
    switches_2 : in std_logic;
    switches_3 : in std_logic;
    switches_4 : in std_logic;
    leds_1 : out std_logic;
    leds_2 : out std_logic;
    leds_3 : out std_logic;
    leds_4 : out std_logic);
end entity Switches_To_LEDs;

architecture RTL of Switches_To_LEDs is
begin
  leds_1 <= switches_1;
  leds_2 <= switches_2;
  leds_3 <= switches_3;
  leds_4 <= switches_4;
end RTL;