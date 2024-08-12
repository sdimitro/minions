# Synthesis
synthesis: switches_to_leds.vhd
	ghdl -a switches_to_leds.vhdl
	yosys -p 'ghdl switches_to_leds; synth_ice40 -json switches_to_leds.json' switches_to_leds.vhd
# yosys -p "read_verilog switches_to_leds.v; synth_ice40 -blif switches_to_leds.blif"

# Place-And-Route [iCE40HX1K - VQ100 (as seen on chip)]
pnr: switches_to_leds.pcf switches_to_leds.json
	nextpnr-ice40 --hx1k --package vq100 --json switches_to_leds.json  --pcf switches_to_leds.pcf  --asc switches_to_leds.asc
# nextpnr-ice40 --hx1k --pcf switches_to_leds.pcf --package vq100 switches_to_leds.blif

# Generate Binary Bitstream
bitstream: switches_to_leds.asc
	icepack switches_to_leds.asc switches_to_leds.bin

# Program iCE40HX1K-VQ100
program: switches_to_leds.bin
	iceprog switches_to_leds.bin