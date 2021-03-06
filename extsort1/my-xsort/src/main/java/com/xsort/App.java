package com.xsort;

import java.util.List;
import java.io.File;
import java.io.IOException;

public class App {
    public static void main(String[] args) {
        if (args.length < 1) {
	    System.out.println("error: no input file as an argument!");
	    System.exit(-1);
	}
        System.out.println("Sorting " + args[0] + " ...");
	try {
	    List<File> intermediates = XSort.split(new File(args[0]));
	    XSort.mergeIntermediates(intermediates,
			             new File(args[0] + ".sorted"));
	} catch (IOException ioe) { System.out.println(ioe); }
    }
}
