package com.xsort;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.Comparator;
import java.util.List;
import java.util.PriorityQueue;
import java.util.zip.Deflater;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/**
 * Goal: offer a generic external-memory sorting program in Java.
 *
 * It must be : - hackable (easy to adapt) - scalable to large files - sensibly
 * efficient.
 *
 * This software is in the public domain.
 *
 * Usage: java com/google/code/externalsorting/XSort somefile.txt out.txt
 *
 * You can change the default maximal number of temporary files with the -t
 * flag: java com/google/code/externalsorting/XSort somefile.txt out.txt
 * -t 3
 *
 * For very large files, you might want to use an appropriate flag to allocate
 * more memory to the Java VM: java -Xms2G
 * com/google/code/externalsorting/XSort somefile.txt out.txt
 *
 * By (in alphabetical order) Philippe Beaudoin, Eleftherios Chetzakis, Jon
 * Elsas, Christan Grant, Daniel Haran, Daniel Lemire, Sugumaran Harikrishnan,
 * Amit Jain, Thomas Mueller, Jerry Yang, First published: April 2010 originally posted at
 * http://lemire.me/blog/archives/2010/04/01/external-memory-sorting-in-java/
 */
public class XSort {


	/* String size estimation settings - BEGIN */
	private static int OBJ_HEADER;
	private static int ARR_HEADER;
	private static int INT_FIELDS = 12;
	private static int OBJ_REF;
	private static int OBJ_OVERHEAD;
	private static boolean IS_64_BIT_JVM;
	static {
		// By default we assume 64 bit JVM
		// (defensive approach since we will get
		// larger estimations in case we are not sure)
		IS_64_BIT_JVM = true;
		// check the system property "sun.arch.data.model"
		// not very safe, as it might not work for all JVM implementations
		// nevertheless the worst thing that might happen is that the JVM is 32bit
		// but we assume its 64bit, so we will be counting a few extra bytes per string object
		// no harm done here since this is just an approximation.
		String arch = System.getProperty("sun.arch.data.model");
		if (arch != null) {
			if (arch.indexOf("32") != -1) {
				// If exists and is 32 bit then we assume a 32bit JVM
				IS_64_BIT_JVM = false;
			}
		}
		// The sizes below are a bit rough as we don't take into account
		// advanced JVM options such as compressed oops
		// however if our calculation is not accurate it'll be a bit over
		// so there is no danger of an out of memory error because of this.
		OBJ_HEADER = IS_64_BIT_JVM ? 16 : 8;
		ARR_HEADER = IS_64_BIT_JVM ? 24 : 12;
		OBJ_REF = IS_64_BIT_JVM ? 8 : 4;
		OBJ_OVERHEAD = OBJ_HEADER + INT_FIELDS + OBJ_REF + ARR_HEADER;
	}

	/**
	 * Estimates the size of a {@link String} object in bytes.
	 *
	 * @param s The string to estimate memory footprint.
	 * @return The <strong>estimated</strong> size in bytes.
	 */
	public static long estimatedSizeOf(String s) {
		return (s.length() * 2) + OBJ_OVERHEAD;
	}
	/* String size estimation settings - END */

	/* XSort - BEGIN */

        public static void displayUsage() {
                System.out
                        .println("java com.xsort.XSort inputfile outputfile");
                System.out.println("Flags are:");
                System.out.println("-v or --verbose: verbose output");
                System.out.println("-d or --distinct: prune duplicate lines");
                System.out
                        .println("-t or --maxtmpfiles (followed by an integer): specify an upper bound on the number of temporary files");
                System.out
                        .println("-c or --charset (followed by a charset code): specify the character set to use (for sorting)");
                System.out
                        .println("-z or --gzip: use compression for the temporary files");
                System.out
                        .println("-H or --header (followed by an integer): ignore the first few lines");
                System.out
                        .println("-s or --store (following by a path): where to store the temporary files");
                System.out.println("-h or --help: display this message");
        }

        /**
         * This method calls the garbage collector and then returns the free
         * memory. This avoids problems with applications where the GC hasn't
         * reclaimed memory and reports no available memory.
         *
         * @return available memory
         */
        public static long estimateAvailableMemory() {
                System.gc();
                return Runtime.getRuntime().freeMemory();
        }

        /**
         * we divide the file into small blocks. If the blocks are too small, we
         * shall create too many temporary files. If they are too big, we shall
         * be using too much memory.
         *
         * @param sizeoffile
         *                how much data (in bytes) can we expect
         * @param maxtmpfiles
         *                how many temporary files can we create (e.g., 1024)
         * @param maxMemory
         *                Maximum memory to use (in bytes)
         * @return the estimate
         */
        public static long estimateBestSizeOfBlocks(final long sizeoffile,
                final int maxtmpfiles, final long maxMemory) {
                // we don't want to open up much more than maxtmpfiles temporary
                // files, better run
                // out of memory first.
                long blocksize = sizeoffile / maxtmpfiles
                        + (sizeoffile % maxtmpfiles == 0 ? 0 : 1);

                // on the other hand, we don't want to create many temporary
                // files
                // for naught. If blocksize is smaller than half the free
                // memory, grow it.
                if (blocksize < maxMemory / 2) {
                        blocksize = maxMemory / 2;
                }
                return blocksize;
        }

        /**
         * @param args
         * @throws IOException
         */
        //public static void main(final String[] args) throws IOException {
        //        boolean verbose = false;
        //        boolean distinct = false;
        //        int maxtmpfiles = DEFAULTMAXTEMPFILES;
        //        Charset cs = Charset.defaultCharset();
        //        String inputfile = null, outputfile = null;
        //        File tempFileStore = null;
        //        boolean usegzip = false;
        //        int headersize = 0;
        //        for (int param = 0; param < args.length; ++param) {
        //                if (args[param].equals("-v")
        //                        || args[param].equals("--verbose")) {
        //                        verbose = true;
        //                } else if ((args[param].equals("-h") || args[param]
        //                        .equals("--help"))) {
        //                        displayUsage();
        //                        return;
        //                } else if ((args[param].equals("-d") || args[param]
        //                        .equals("--distinct"))) {
        //                        distinct = true;
        //                } else if ((args[param].equals("-t") || args[param]
        //                        .equals("--maxtmpfiles"))
        //                        && args.length > param + 1) {
        //                        param++;
        //                        maxtmpfiles = Integer.parseInt(args[param]);
        //                        if (headersize < 0) {
        //                                System.err
        //                                        .println("maxtmpfiles should be positive");
        //                        }
        //                } else if ((args[param].equals("-c") || args[param]
        //                        .equals("--charset"))
        //                        && args.length > param + 1) {
        //                        param++;
        //                        cs = Charset.forName(args[param]);
        //                } else if ((args[param].equals("-z") || args[param]
        //                        .equals("--gzip"))) {
        //                        usegzip = true;
        //                } else if ((args[param].equals("-H") || args[param]
        //                        .equals("--header")) && args.length > param + 1) {
        //                        param++;
        //                        headersize = Integer.parseInt(args[param]);
        //                        if (headersize < 0) {
        //                                System.err
        //                                        .println("headersize should be positive");
        //                        }
        //                } else if ((args[param].equals("-s") || args[param]
        //                        .equals("--store")) && args.length > param + 1) {
        //                        param++;
        //                        tempFileStore = new File(args[param]);
        //                } else {
        //                        if (inputfile == null)
        //                                inputfile = args[param];
        //                        else if (outputfile == null)
        //                                outputfile = args[param];
        //                        else
        //                                System.out.println("Unparsed: "
        //                                        + args[param]);
        //                }
        //        }
        //        if (outputfile == null) {
        //                System.out
        //                        .println("please provide input and output file names");
        //                displayUsage();
        //                return;
        //        }
        //        Comparator<String> comparator = defaultcmp;
        //        List<File> l = sortInBatch(new File(inputfile), comparator,
        //                maxtmpfiles, cs, tempFileStore, distinct, headersize,
        //                usegzip);
        //        if (verbose)
        //                System.out
        //                        .println("created " + l.size() + " tmp files");
        //        mergeSortedFiles(l, new File(outputfile), comparator, cs,
        //                distinct, false, usegzip);
        //}

        /**
         * This merges several BinaryFileBuffer to an output writer.
         *
         * @param fbw
         *                A buffer where we write the data.
         * @param cmp
         *                A comparator object that tells us how to sort the
         *                lines.
         * @param distinct
         *                Pass <code>true</code> if duplicate lines should be
         *                discarded. (elchetz@gmail.com)
         * @param buffers
         *                Where the data should be read.
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         *
         */
        public static int mergeSortedFiles(BufferedWriter fbw,
                final Comparator<String> cmp, boolean distinct,
                List<BinaryFileBuffer> buffers) throws IOException {
                PriorityQueue<BinaryFileBuffer> pq = new PriorityQueue<BinaryFileBuffer>(
                        11, new Comparator<BinaryFileBuffer>() {
                                @Override
                                public int compare(BinaryFileBuffer i,
                                        BinaryFileBuffer j) {
                                        return cmp.compare(i.peek(), j.peek());
                                }
                        });
                for (BinaryFileBuffer bfb : buffers)
                        if (!bfb.empty())
                                pq.add(bfb);
                int rowcounter = 0;
                try {
                        if(!distinct) {
                            while (pq.size() > 0) {
                                    BinaryFileBuffer bfb = pq.poll();
                                    String r = bfb.pop();
                                    fbw.write(r);
                                    fbw.newLine();
                                    ++rowcounter;
                                    if (bfb.empty()) {
                                            bfb.fbr.close();
                                    } else {
                                            pq.add(bfb); // add it back
                                    }
                            }
                        } else {                                String lastLine = null;
                            if(pq.size() > 0) {
                     			BinaryFileBuffer bfb = pq.poll();
                     			lastLine = bfb.pop();
                     			fbw.write(lastLine);
                     			fbw.newLine();
                     			++rowcounter;
                     			if (bfb.empty()) {
                     				bfb.fbr.close();
                     			} else {
                     				pq.add(bfb); // add it back
                     			}
                     		}
                            while (pq.size() > 0) {
                    			BinaryFileBuffer bfb = pq.poll();
                    			String r = bfb.pop();
                    			// Skip duplicate lines
                    			if  (cmp.compare(r, lastLine) != 0) {
                    				fbw.write(r);
                    				fbw.newLine();
                    				lastLine = r;
                    			}
                    			++rowcounter;
                    			if (bfb.empty()) {
                    				bfb.fbr.close();
                    			} else {
                    				pq.add(bfb); // add it back
                    			}
                            }
                        }
                } finally {
                        fbw.close();
                        for (BinaryFileBuffer bfb : pq)
                                bfb.close();
                }
                return rowcounter;

        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         *                files to be merged
         * @param outputfile
         *                output file
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         */
        public static int mergeSortedFiles(List<File> files, File outputfile)
                throws IOException {
                return mergeSortedFiles(files, outputfile, defaultcmp,
                        Charset.defaultCharset());
        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         * @param outputfile
         * @param cmp
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         */
        public static int mergeSortedFiles(List<File> files, File outputfile,
                final Comparator<String> cmp) throws IOException {
                return mergeSortedFiles(files, outputfile, cmp,
                        Charset.defaultCharset());
        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         * @param outputfile
         * @param cmp
         * @param distinct
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         */
        public static int mergeSortedFiles(List<File> files, File outputfile,
                final Comparator<String> cmp, boolean distinct)
                throws IOException {
                return mergeSortedFiles(files, outputfile, cmp,
                        Charset.defaultCharset(), distinct);
        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         * @param outputfile
         * @param cmp
         * @param cs
         *                character set to use to load the strings
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         */
        public static int mergeSortedFiles(List<File> files, File outputfile,
                final Comparator<String> cmp, Charset cs) throws IOException {
                return mergeSortedFiles(files, outputfile, cmp, cs, false);
        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         *                The {@link List} of sorted {@link File}s to be merged.
         * @param distinct
         *                Pass <code>true</code> if duplicate lines should be
         *                discarded. (elchetz@gmail.com)
         * @param outputfile
         *                The output {@link File} to merge the results to.
         * @param cmp
         *                The {@link Comparator} to use to compare
         *                {@link String}s.
         * @param cs
         *                The {@link Charset} to be used for the byte to
         *                character conversion.
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         * @since v0.1.2
         */
        public static int mergeSortedFiles(List<File> files, File outputfile,
                final Comparator<String> cmp, Charset cs, boolean distinct)
                throws IOException {
                return mergeSortedFiles(files, outputfile, cmp, cs, distinct,
                        false, false);
        }

        /**
         * This merges a bunch of temporary flat files
         *
         * @param files
         *                The {@link List} of sorted {@link File}s to be merged.
         * @param distinct
         *                Pass <code>true</code> if duplicate lines should be
         *                discarded. (elchetz@gmail.com)
         * @param outputfile
         *                The output {@link File} to merge the results to.
         * @param cmp
         *                The {@link Comparator} to use to compare
         *                {@link String}s.
         * @param cs
         *                The {@link Charset} to be used for the byte to
         *                character conversion.
         * @param append
         *                Pass <code>true</code> if result should append to
         *                {@link File} instead of overwrite. Default to be false
         *                for overloading methods.
         * @param usegzip
         *                assumes we used gzip compression for temporary files
         * @return The number of lines sorted. (P. Beaudoin)
         * @throws IOException
         * @since v0.1.4
         */
        public static int mergeSortedFiles(List<File> files, File outputfile,
                final Comparator<String> cmp, Charset cs, boolean distinct,
                boolean append, boolean usegzip) throws IOException {
                ArrayList<BinaryFileBuffer> bfbs = new ArrayList<BinaryFileBuffer>();
                for (File f : files) {
                        final int BUFFERSIZE = 2048;
                        InputStream in = new FileInputStream(f);
                        BufferedReader br;
                        if (usegzip) {
                                br = new BufferedReader(
                                        new InputStreamReader(
                                                new GZIPInputStream(in,
                                                        BUFFERSIZE), cs));
                        } else {
                                br = new BufferedReader(new InputStreamReader(
                                        in, cs));
                        }

                        BinaryFileBuffer bfb = new BinaryFileBuffer(br);
                        bfbs.add(bfb);
                }
                BufferedWriter fbw = new BufferedWriter(new OutputStreamWriter(
                        new FileOutputStream(outputfile, append), cs));
                int rowcounter = mergeSortedFiles(fbw, cmp, distinct, bfbs);
                for (File f : files)
                        f.delete();
                return rowcounter;
        }

        /**
         * This sorts a file (input) to an output file (output) using default
         * parameters
         *
         * @param input
         *                source file
         *
         * @param output
         *                output file
         * @throws IOException
         */
        public static void sort(final File input, final File output)
                throws IOException {
                XSort.mergeSortedFiles(XSort.sortInBatch(input),
                        output);
        }

        /**
         * Sort a list and save it to a temporary file
         *
         * @return the file containing the sorted data
         * @param tmplist
         *                data to be sorted
         * @param cmp
         *                string comparator
         * @param cs
         *                charset to use for output (can use
         *                Charset.defaultCharset())
         * @param tmpdirectory
         *                location of the temporary files (set to null for
         *                default location)
         * @throws IOException
         */
        public static File sortAndSave(List<String> tmplist,
                Comparator<String> cmp, Charset cs, File tmpdirectory)
                throws IOException {
                return sortAndSave(tmplist, cmp, cs, tmpdirectory, false, false);
        }

        /**
         * Sort a list and save it to a temporary file
         *
         * @return the file containing the sorted data
         * @param tmplist
         *                data to be sorted
         * @param cmp
         *                string comparator
         * @param cs
         *                charset to use for output (can use
         *                Charset.defaultCharset())
         * @param tmpdirectory
         *                location of the temporary files (set to null for
         *                default location)
         * @param distinct
         *                Pass <code>true</code> if duplicate lines should be
         *                discarded.
         * @param usegzip
         *                set to true if you are using gzip compression for the
         *                temporary files
         * @throws IOException
         */
        public static File sortAndSave(List<String> tmplist,
                Comparator<String> cmp, Charset cs, File tmpdirectory,
                boolean distinct, boolean usegzip) throws IOException {
                Collections.sort(tmplist, cmp);// In Java8, we can do tmplist = tmplist.parallelStream().sorted(cmp).collect(Collectors.toCollection(ArrayList<String>::new));
                File newtmpfile = File.createTempFile("sortInBatch",
                        "flatfile", tmpdirectory);
                newtmpfile.deleteOnExit();
                OutputStream out = new FileOutputStream(newtmpfile);
                int ZIPBUFFERSIZE = 2048;
                if (usegzip)
                        out = new GZIPOutputStream(out, ZIPBUFFERSIZE) {
                                {
                                        this.def.setLevel(Deflater.BEST_SPEED);
                                }
                        };
                BufferedWriter fbw = new BufferedWriter(new OutputStreamWriter(
                        out, cs));
                try {
                        if (!distinct) {
                            for (String r : tmplist) {
                                        fbw.write(r);
                                        fbw.newLine();
                            }
                        } else {
                    		String lastLine = null;
                    		Iterator<String> i = tmplist.iterator();
                    		if(i.hasNext()) {
                    			lastLine = i.next();
                    			fbw.write(lastLine);
                  				fbw.newLine();
                    		}
                    		while (i.hasNext()) {
                    			String r = i.next();
                    			// Skip duplicate lines
                    			if (cmp.compare(r, lastLine) != 0) {
                    				fbw.write(r);
                    				fbw.newLine();
                    				lastLine = r;
                    			}
                    		}
                        }
                } finally {
                        fbw.close();
                }
                return newtmpfile;
        }

        /**
         * @param fbr
         *                data source
         * @param datalength
         *                estimated data volume (in bytes)
         * @param cmp
         *                string comparator
         * @param maxtmpfiles
         *                maximal number of temporary files
         * @param maxMemory
         *                maximum amount of memory to use (in bytes)
         * @param cs
         *                character set to use (can use
         *                Charset.defaultCharset())
         * @param tmpdirectory
         *                location of the temporary files (set to null for
         *                default location)
         * @param distinct
         *                Pass <code>true</code> if duplicate lines should be
         *                discarded.
         * @param numHeader
         *                number of lines to preclude before sorting starts
         * @param usegzip
         *                use gzip compression for the temporary files
         * @return a list of temporary flat files
         * @throws IOException
         */
        public static List<File> sortInBatch(final BufferedReader fbr,
                final long datalength, final Comparator<String> cmp,
                final int maxtmpfiles, long maxMemory, final Charset cs,
                final File tmpdirectory, final boolean distinct,
                final int numHeader, final boolean usegzip) throws IOException {
                List<File> files = new ArrayList<File>();
                long blocksize = estimateBestSizeOfBlocks(datalength,
                        maxtmpfiles, maxMemory);// in
                // bytes

                try {
                        List<String> tmplist = new ArrayList<String>();
                        String line = "";
                        try {
                                int counter = 0;
                                while (line != null) {
                                        long currentblocksize = 0;// in bytes
                                        while ((currentblocksize < blocksize)
                                                && ((line = fbr.readLine()) != null)) {
                                                // as long as you have enough
                                                // memory
                                                if (counter < numHeader) {
                                                        counter++;
                                                        continue;
                                                }
                                                tmplist.add(line);
                                                currentblocksize += estimatedSizeOf(line);
                                        }
                                        files.add(sortAndSave(tmplist, cmp, cs,
                                                tmpdirectory, distinct, usegzip));
                                        tmplist.clear();
                                }
                        } catch (EOFException oef) {
                                if (tmplist.size() > 0) {
                                        files.add(sortAndSave(tmplist, cmp, cs,
                                                tmpdirectory, distinct, usegzip));
                                        tmplist.clear();
                                }
                        }
                } finally {
                        fbr.close();
                }
                return files;
        }

        /**
	 * [STD]
         */
        public static List<File> sortInBatch(File file) throws IOException {
		Charset cs = Charset.defaultCharset();
		Comparator<String> cmp = defaultcmp;
		int maxtmpfiles = DEFAULTMAXTEMPFILES;
		File tmpdirectory = null;
		boolean distinct = false;
		int numHeader = 0;
		boolean usegzip = false;
		BufferedReader fbr = new BufferedReader(new InputStreamReader(
			new FileInputStream(file), cs));
		return sortInBatch(fbr, file.length(), cmp, maxtmpfiles,
			estimateAvailableMemory(), cs, tmpdirectory, distinct,
			numHeader, usegzip);
        }

        /**
         * default comparator between strings.
         */
        public static Comparator<String> defaultcmp = new Comparator<String>() {
                @Override
                public int compare(String r1, String r2) {
                        return r1.compareTo(r2);
                }
        };

        /**
         * Default maximal number of temporary files allowed.
         */
        public static final int DEFAULTMAXTEMPFILES = 1024;
	/* XSort - END */
}

/**
 * This is essentially a thin wrapper on top of a BufferedReader... which keeps
 * the last line in memory.
 *
 * @author Daniel Lemire
 */
final class BinaryFileBuffer {
        public BinaryFileBuffer(BufferedReader r) throws IOException {
                this.fbr = r;
                reload();
        }
        public void close() throws IOException {
                this.fbr.close();
        }

        public boolean empty() {
                return this.cache == null;
        }

        public String peek() {
                return this.cache;
        }

        public String pop() throws IOException {
                String answer = peek().toString();// make a copy
                reload();
                return answer;
        }

        private void reload() throws IOException {
                this.cache = this.fbr.readLine();
        }

        public BufferedReader fbr;

        private String cache;

}
