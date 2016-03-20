package com.xsort;

import java.io.*;
import java.util.*;
import java.util.stream.*;
import java.util.zip.*;
import java.nio.charset.Charset;

public class XSort {


	/* Runtime Parameters - BEGIN */
	// Java Object header size
	private static int OBJ_HDR;
	// Java Canal header size
	private static int ARR_HDR;
	// Java Default INT Field Storage size
	private static int INT_FIELDS = 12;
	// JVM Pointer Storage size
	private static int OBJ_PTR;
	// Total size of Object Overhead
	private static int OBJ_OVERHEAD;
	// 64-bit JVM flag
	private static boolean FLAG_64_BIT;

	static {
		// Assuming 64-bit initially
		FLAG_64_BIT = true;
		// Then checking system at runtime for actual architecture
		// NOTE: Even though some JVM implementations do not support
		// the property below, at worst case we slightly overcount
		// the approximate size of each string object, which is ok
		String arch = System.getProperty("sun.arch.data.model");
		if (arch != null) {
			if (arch.indexOf("32") != -1) {
				FLAG_64_BIT = false;
			}
		}
		// Now calculating the rest of the runtime parameters.
		OBJ_HDR = FLAG_64_BIT ? 16 : 8;
		ARR_HDR = FLAG_64_BIT ? 24 : 12;
		OBJ_PTR = FLAG_64_BIT ? 8  : 4;
		OBJ_OVERHEAD = OBJ_HDR + INT_FIELDS + OBJ_PTR + ARR_HDR;
		// In the case of byte inaccuracy we are still ok
		// since we can only overcount which means that we
		// still avoid OutOfMemoryErrors.
	}
	/* Runtime Parameters - END */

	/* XSort - BEGIN */

	/* == DEFAULT CONFIGURATION - BEGIN == */
	// CONFIG - Maximum number of temporary files allowed
        public static final int DEFAULTMAXTEMPFILES = 1024;
	// CONFIG - Comparator between strings
        public static final Comparator<String> DEFAULTCMP
		= new Comparator<String>() {
                @Override
                public int compare(String s1, String s2)
		{ return s1.compareTo(s2); }
        };
	// CONFIG - Directory for intermediate files (null means binary root) 
	public static final File DEFAULTTMPDIR = null;
	// CONFIG - Gzip flag (set true for enabling compression)
	public static final boolean GZIPFLAG = false;
	// CONFIG - If Gzip enabled, set ZIP Bufffer size
        public static final int ZIPBUFFERSIZE = 2048;
	// CONFIG - Append to original flag
	public static final boolean APPENDFLAG = false;
	/* == DEFAULT CONFIGURATION - END == */

	/* == ESTIMATION HELPER FUNCTIONS - BEGIN == */
	// Estimate the size of given String in bytes
	public static long estSizeOf(String s)
	{ return OBJ_OVERHEAD + (s.length() * 2); }

	// Initiate GC in an attempt to avoid OutOfMemoryErrors
	// and return available memory
        public static long estAvailMem()
	{ System.gc(); return Runtime.getRuntime().freeMemory(); }

	// Estimate block size when dividing file into blocks.
	// Effect: Small files -> Spit out many intermediate files
	// Effect: Big files   -> Use a lot of memory
        public static long estBestBlockSize(final long fileSize,
			final int maxTmpFiles, final long maxMem) {
		// Attempt to open less MaxTmpFiles (may give OOM error)
                long blockSize = fileSize / maxTmpFiles
                        + (fileSize % maxTmpFiles == 0 ? 0 : 1);

		// If block less than halft the available memory, grow it
                return (blockSize < maxMem / 2) ? maxMem / 2 : blockSize;
        }
	/* == ESTIMATION HELPER FUNCTIONS - END == */

	/* == SPLIT TO INTERMEDIATE AND SORT - BEGIN == */
        private static File saveSortedTemp(List<String> lines)
		throws IOException {

                lines = lines.parallelStream().sorted(DEFAULTCMP).collect(
				Collectors.toCollection(ArrayList<String>::new));

                File tmpFile = File.createTempFile("intermediate",
                        "flat", DEFAULTTMPDIR);
                tmpFile.deleteOnExit();

                OutputStream fos = new FileOutputStream(tmpFile);
                if (GZIPFLAG) {
                        fos = new GZIPOutputStream(fos, ZIPBUFFERSIZE) {
                                {
                                        this.def.setLevel(Deflater.BEST_SPEED);
                                }
                        };
		}
                BufferedWriter bw =
			new BufferedWriter(new OutputStreamWriter(fos));
                try {
		    for (String r : lines)
		    { bw.write(r); bw.newLine(); }
                } finally { bw.close(); }
                return tmpFile;
        }


        public static List<File> split(File file) throws IOException {
		// Split file to intermediate sorted files
		// by reading the given file line by line.
		// Before reading the file check how much
		// available memory you have and create
		// chucks according to it. Keep only one
		// chuck in memory at a time and also keep
		// track of all the chunk (flat file) names
		// in order to return them for future merging

                List<File> files = new ArrayList<File>();

		BufferedReader br = new BufferedReader(new InputStreamReader(
			new FileInputStream(file)));
		long maxMemory = estAvailMem();
                long blockSize = estBestBlockSize(file.length(),
				DEFAULTMAXTEMPFILES, maxMemory);

                try {
                        List<String> lines = new ArrayList<String>();
                        String line = "";
                        try {
                                while (line != null) {
                                        long currentBlockSize = 0;
                                        while (currentBlockSize < blockSize
                                               &&
					       (line = br.readLine()) != null) {
                                                lines.add(line);
                                                currentBlockSize += estSizeOf(line);
                                        }
                                        files.add(saveSortedTemp(lines));
                                        lines.clear();
                                }
                        } catch (EOFException eofe) {
                                if (lines.size() > 0) {
                                        files.add(saveSortedTemp(lines));
                                        lines.clear();
                                }
                        }
                } finally { br.close(); }
                return files;
        }
	/* == SPLIT TO INTERMEDIATE AND SORT - END == */

        private static int mergeByBFCs(BufferedWriter bw,
                List<BFC> bfcs) throws IOException {
		// Keep the Buffered File Caches sorted by their
		// contents (their last line). Remove any empty
		// caches. Then start flushing each cache to
		// write to the final file in a sorted manner.
		// At each flush check if the cache does not
		// have any other values (which means that the
		// intermediate file was fully read) and insert
		// it back with the rest of the Buffered File
		// Caches. (Optionally: return number of records
		// sorted)

                PriorityQueue<BFC> cachePQ = new PriorityQueue<BFC>(
                        11, new Comparator<BFC>() {
                                @Override
                                public int compare(BFC a, BFC b) {
                                        return DEFAULTCMP.compare(a.access(),
							          b.access());
                                }});

                for (BFC bfc : bfcs) { if (!bfc.empty()) cachePQ.add(bfc); }

                int rowcounter = 0;
                try {
		    while (cachePQ.size() > 0) {
			    BFC bfc = cachePQ.poll();
			    String l = bfc.flush();

			    bw.write(l); bw.newLine(); ++rowcounter;

			    if (bfc.empty()) { bfc.close(); }
			    else { cachePQ.add(bfc); }
		    }
                } finally { bw.close(); for (BFC bfc : cachePQ) bfc.close(); }
                return rowcounter;

        }

        public static int mergeIntermediates(List<File> files, File out)
                throws IOException {
		// For each of the intermediates open a BufferedReader
		// with a Buffered File Cache. Then open a BufferedWriter
		// stream for the final merging. Execute the final merging
		// and delete all the intermediate files that are not
		// needed anymore. (Optionally: return number of rows

                ArrayList<BFC> bfcs = new ArrayList<BFC>();
                for (File f : files) {
                        InputStream is = new FileInputStream(f);
                        BufferedReader br = new BufferedReader(new InputStreamReader(
							is));
                        if (GZIPFLAG) {
                                br = new BufferedReader(
                                        new InputStreamReader(
                                                new GZIPInputStream(is,
                                                        ZIPBUFFERSIZE)));
                        }
			bfcs.add(new BFC(br));
                }

                BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(
                        new FileOutputStream(out, APPENDFLAG)));
                int rowcounter = mergeByBFCs(bw, bfcs);
                for (File f : files) { f.delete(); }
                return rowcounter;
        }
	/* == MERGE SORTED INTERMEDIATES TO FINAL - END == */
	/* XSort - END */
}

/*
 * BFC - Buffered File Cache:
 * An abstraction based on BufferedReader
 * which caches the last line read.
 */
final class BFC {
        public BufferedReader br;
        private String cache;

        public BFC(BufferedReader r) throws IOException
	{ this.br = r; load(); }

        public void close() throws IOException
	{ this.br.close(); }

        public boolean empty()
	{ return this.cache == null; }

        public String access()
	{ return this.cache; }

        public String flush() throws IOException
	{ String retval = access().toString(); load(); return retval; }

        private void load() throws IOException
	{ this.cache = this.br.readLine(); }
}

