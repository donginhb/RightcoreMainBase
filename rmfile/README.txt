RMFILE

This program can be used to unlink those nasty files that you may not be able to
unlink (delete) with the RM program.

Synopsis:
$ rmfile [<file(s)> ....] [-af <afile>] [-d[=<delay>]] [-r] [-z] [-Q] [-V]

Arguments:
<file(s)>	filename(s) to delete
-af <afile>	argument list file
-d=<delay>	specifiy a delay ('delay') to wait before deleting the file
-r		act recursively on directories
-z		ignore incident of no arguments
-Q		do not complain about certain things happening
-V		print program version to standard-error and then exit

Example usage:
$ rmfile -f "<filename>" [-af <afile>]

This program can also unlink files at a delayed time from invocation
with something like:

$ rmfile -d <filename>

for a default delay or:

$ rmfile -d=<delay> <filename>

for a specific delay <delay>.

