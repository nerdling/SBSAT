SUBDIRS=manual
EXTRA_DIST=ftest.c arch.txt Doxyfile ite.1 platforms.txt

ite.1: 
	help2man ../src/ite > ite.1

doc:
	doxygen
