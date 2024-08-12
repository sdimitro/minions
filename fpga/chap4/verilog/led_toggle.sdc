# The Synopsys Design Constraints (SDC) format is used to specify the design intent,
# including timing, power and area constraints for a design.

# Create clock with 40 ns period (25MHz frequency)
create_clock -period 40.00 -name {i_clk}
