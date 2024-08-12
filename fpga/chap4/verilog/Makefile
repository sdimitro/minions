# Synthesis
synthesis: led_toggle.v led_toggle.ys
	yosys -m sdc led_toggle.ys

# Place-And-Route [iCE40HX1K - VQ100 (as seen on chip)]
pnr: led_toggle.pcf led_toggle.json
	nextpnr-ice40 --hx1k --package vq100 --json led_toggle.json  --pcf led_toggle.pcf  --asc led_toggle.asc

# Generate Binary Bitstream
bitstream: led_toggle.asc
	icepack led_toggle.asc led_toggle.bin

# Program iCE40HX1K-VQ100
program: led_toggle.bin
	iceprog led_toggle.bin