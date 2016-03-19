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
	// CONFIG - Character set
	public static final Charset DEFAULTCS = Charset.defaultCharset();
	// CONFIG - Direcotry for intermediate files (null means binary root) 
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

        /**
         * This merges several BFC to an output writer.
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
                List<BFC> buffers) throws IOException {
                PriorityQueue<BFC> pq = new PriorityQueue<BFC>(
                        11, new Comparator<BFC>() {
                                @Override
                                public int compare(BFC i,
                                        BFC j) {
                                        return cmp.compare(i.access(), j.access());
                                }
                        });
                for (BFC bfb : buffers)
                        if (!bfb.empty())
                                pq.add(bfb);
                int rowcounter = 0;
                try {
                        if(!distinct) {
                            while (pq.size() > 0) {
                                    BFC bfb = pq.poll();
                                    String r = bfb.flush();
                                    fbw.write(r);
                                    fbw.newLine();
                                    ++rowcounter;
                                    if (bfb.empty()) {
                                            bfb.close();
                                    } else {
                                            pq.add(bfb); // add it back
                                    }
                            }
                        } else {                                String lastLine = null;
                            if(pq.size() > 0) {
                     			BFC bfb = pq.poll();
                     			lastLine = bfb.flush();
                     			fbw.write(lastLine);
                     			fbw.newLine();
                     			++rowcounter;
                     			if (bfb.empty()) {
                     				bfb.close();
                     			} else {
                     				pq.add(bfb); // add it back
                     			}
                     		}
                            while (pq.size() > 0) {
                    			BFC bfb = pq.poll();
                    			String r = bfb.flush();
                    			// Skip duplicate lines
                    			if  (cmp.compare(r, lastLine) != 0) {
                    				fbw.write(r);
                    				fbw.newLine();
                    				lastLine = r;
                    			}
                    			++rowcounter;
                    			if (bfb.empty()) {
                    				bfb.close();
                    			} else {
                    				pq.add(bfb); // add it back
                    			}
                            }
                        }
                } finally {
                        fbw.close();
                        for (BFC bfb : pq)
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
                return mergeSortedFiles(files, outputfile, DEFAULTCMP,
				DEFAULTCS, false, APPENDFLAG, GZIPFLAG);
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
                ArrayList<BFC> bfbs = new ArrayList<BFC>();
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

                        BFC bfb = new BFC(br);
                        bfbs.add(bfb);
                }
                BufferedWriter fbw = new BufferedWriter(new OutputStreamWriter(
                        new FileOutputStream(outputfile, append), cs));
                int rowcounter = mergeSortedFiles(fbw, cmp, distinct, bfbs);
                for (File f : files)
                        f.delete();
                return rowcounter;
        }

        public static File saveSortedTemp(List<String> lines) throws IOException {
		// Sort and save intermediate file

                // In Java 8, we can do lines = lines.parallelStream().sorted(cmp).collect(Collectors.toCollection(ArrayList<String>::new));
		Collections.sort(lines, DEFAULTCMP);
		//
                File tmpFile = File.createTempFile("intermediate",
                        "flat", DEFAULTTMPDIR);
                tmpFile.deleteOnExit();

                OutputStream out = new FileOutputStream(tmpFile);
                if (GZIPFLAG) {
                        out = new GZIPOutputStream(out, ZIPBUFFERSIZE) {
                                {
                                        this.def.setLevel(Deflater.BEST_SPEED);
                                }
                        };
		}
                BufferedWriter bw =
			new BufferedWriter(new OutputStreamWriter(out, DEFAULTCS));
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
			new FileInputStream(file), DEFAULTCS));
		long maxMemory = estAvailMem();
                long blockSize = estBestBlockSize(file.length(),
				DEFAULTMAXTEMPFILES, maxMemory);

                try {
                        List<String> lines = new ArrayList<String>();
                        String line = "";
                        try {
                                while (line != null) {
                                        long currentBlockSize = 0;
                                        while ((currentBlockSize < blockSize)
                                                && ((line = br.readLine()) != null)) {
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

