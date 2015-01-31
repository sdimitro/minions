# Pretty rudimentary for now

all: sysconf pathconf confstr chutime

sysconf: sysconf.c
	gcc sysconf.c -o sysconf

pathconf: pathconf.c
	gcc pathconf.c -o pathconf

confstr: confstr.c
	gcc confstr.c -o confstr

chutime: chutime.c
	gcc chutime.c -o chutime

clean:
	rm sysconf pathconf confstr chutime
