#!/bin/sh

echo 'Removing old tests'
rm -f test.txt test.txt.sorted
echo 'Generating new tests'
gensort -a 10 test.txt
echo 'Running sort...'
java -cp target/my-xsort-1.0-SNAPSHOT.jar com.xsort.App test.txt
echo 'unix2dos changes...'
unix2dos test.txt.sorted
echo 'Runnning valsort..'
valsort test.txt.sorted
echo 'Done!'
