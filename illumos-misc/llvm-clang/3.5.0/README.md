### Details

#### What I used for the build
* Distribution - OI Hipster
* Compiler - gcc 4.8.3
* Linker - Solaris Link Editors: 5.11-1.1749 (illumos)
* GNU binutils 2.23 for other utilities
* [other requirements from the official website] (http://llvm.org/docs/GettingStarted.html#requirements)

#### Notes on patches
* `gc-section` - Takes out the use of an argument
		that the illumos linker doesn't have. The argument
		is --gc-sections and basically it does unused code
		elimination.
* `paths` - Sets the correct paths in order for the
		build to include the correct solaris and gcc
		4.8.3 files. (Thanks to Alexander Pyhalov)

#### Other notes

The `exported_vars` file contains the environment
variables that should be exported for the build.

The `check-all.report` contains all the errors
from running `gmake check-all` after the build is
finished. These are the regression tests to ensure
that everything is working. If you see a lot of errors
that have to do with `grep` or `mv` it probably
means that you are using the utilities that come
with illumos and not their GNU counterparts. Still,
errors exists and hopefully they will be resolved
in the future. Reminder: to run these tests your
build should have assertions enabled.
