#!/usr/local/bin/python3.4

import sys
from datetime import date as D

if len(sys.argv) < 2:
	print("usage: diffdate <dd:mm:yyyy> [<dd:mm:yyyy>]")
	exit()

date = sys.argv[1].split(":")

yr = int(date[2])
mo = int(date[1])
dy = int(date[0])
date1 = D(yr, mo, dy)

date2 = D.today()

if len(sys.argv) == 3:
	date = sys.argv[2].split(":")

	yr = int(date[2])
	mo = int(date[1])
	dy = int(date[0])

	date2 = D(yr, mo, dy)


print (date2 - date1)
