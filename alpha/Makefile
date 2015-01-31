# Pretty rudimentary for now

all: sysconf pathconf confstr

sysconf: sysconf.c
	gcc sysconf.c -o sysconf

pathconf: pathconf.c
	gcc pathconf.c -o pathconf

confstr: confstr.c
	gcc confstr.c -o confstr

clean:
	rm sysconf pathconf confstr
