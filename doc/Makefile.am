SUBDIRS=manual
EXTRA_DIST=ftest.c arch.txt Doxyfile sbsat.1 platforms.txt

sbsat.1: 
	help2man ../src/sbsat > sbsat.1

doc:
	doxygen
