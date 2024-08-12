all: cas faa faa_threshold scq index_test cycle_test

cycle_test: cycle_test.c
	gcc -Wall -std=c17 $< -o cycle_test

index_test: index_test.c
	gcc -Wall -std=c17 -Wpedantic -Werror $< -o index_test

scq: scq.c
	gcc -Wall -std=c17 -Wpedantic -Werror $< -o scq

cas: cas.c
	gcc -Wall -std=c17 -Wpedantic -Werror $< -o cas

faa: faa.c
	gcc -Wall -std=c17 -Wpedantic -Werror $< -o faa

faa_threshold: faa_threshold.c
	gcc -Wall -std=c17 -Wpedantic -Werror $< -o faa_threshold
