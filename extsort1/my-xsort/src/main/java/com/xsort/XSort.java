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
	// CONFIG - default comparator between strings
        public static final Comparator<String> DEFAULTCMP
		= new Comparator<String>() {
                @Override
                public int compare(String s1, String s2)
		{ return s1.compareTo(s2); }
        };
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
        public static long estimateBestSizeOfBlocks(final long fileSize,
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
		Charset cs = Charset.defaultCharset();
		boolean distinct = false;
		boolean append = false;
		boolean usegzip = false;
		Comparator<String> cmp = DEFAULTCMP;
                return mergeSortedFiles(files, outputfile, cmp,
				cs, distinct, append, usegzip);
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
         * @param br
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
        public static List<File> sortInBatch(final BufferedReader br,
                final long datalength, final Comparator<String> cmp,
                final int maxtmpfiles, long maxMemory, final Charset cs,
                final File tmpdirectory, final boolean distinct,
                final int numHeader, final boolean usegzip) throws IOException {
                List<File> files = new ArrayList<File>();
                long blocksize = estimateBestSizeOfBlocks(datalength,
                        maxtmpfiles, maxMemory); /* in bytes */

                try {
                        List<String> tmplist = new ArrayList<String>();
                        String line = "";
                        try {
                                int counter = 0;
                                while (line != null) {
                                        long currentblocksize = 0;// in bytes
                                        while ((currentblocksize < blocksize)
                                                && ((line = br.readLine()) != null)) {
                                                // as long as you have enough
                                                // memory
                                                if (counter < numHeader) {
                                                        counter++;
                                                        continue;
                                                }
                                                tmplist.add(line);
                                                currentblocksize += estSizeOf(line);
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
                        br.close();
                }
                return files;
        }

        /**
	 * [STD]
         */
        public static List<File> sortInBatch(File file) throws IOException {
		Charset cs = Charset.defaultCharset();
		Comparator<String> cmp = DEFAULTCMP;
		int maxtmpfiles = DEFAULTMAXTEMPFILES;
		File tmpdirectory = null;
		boolean distinct = false;
		int numHeader = 0;
		boolean usegzip = false;
		BufferedReader br = new BufferedReader(new InputStreamReader(
			new FileInputStream(file), cs));
		return sortInBatch(br, file.length(), cmp, maxtmpfiles,
			estAvailMem(), cs, tmpdirectory, distinct,
			numHeader, usegzip);
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

