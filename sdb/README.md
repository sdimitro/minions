# SDB - Another Debugger Built From A Language

SDB is a cheap copy of
[ACID](http://www2.informatik.hu-berlin.de/~apolze/LV/plan9.docs/acidpaper.html)
for Illumos. The debugger itself is written and
can be extended in Lua, and its interface to the
OS and libraries is written in plain C.

## Status

The debugger is currently at a very experimental
state. You are likely to hit many assertions and
bugs.

The features implemented so far are the following:
* SDB can open core dumps & elf objects or attach
  to a live process as its target.
* Multiple targets can be debugged within a single
  instance of SDB at the same time.
* SDB can grab the registers of the current target.
* SDB can obtain process content types available in
  the target.
* SDB can obtain full path to the process's
  executable if that's available.

## Why not MDB?

MDB is an amazing debugger and it has never let me
down. It is the way it is for many valid reasons.
I just thought it would be a cool side-project to
make something more extensible and SDB is me giving
this idea a shot.
