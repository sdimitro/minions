module mux_using_if {

	input din_0, din_1, sel

	output reg mux_out

	::MUX {
	// 	if (sel == 1'b0)
	// 		mux_out = din_0;
	// 	else
	// 		mux_out = din_1;
	}
}
