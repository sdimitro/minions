all:
	gcc -g -O0 -Wall -Wextra -Wpedantic -std=c18 preemptible.c -o preemptible
	gcc -g -O0 -Wall -Wextra -Wpedantic -std=c18 non_preemptible.c -o non_preemptible
	gcc -g -O0 -Wall -Wextra -Wpedantic -std=c18 non_preemptible1.c -o non_preemptible1


test_preemptible:
	perf stat ./preemptible

test_non_preemptible:
	perf stat ./non_preemptible
