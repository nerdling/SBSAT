dist_man_MANS=sbsat.1

sbsat.1: $(top_srcdir)/src/utils/params.cc $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) -C ../../src sbsat$(EXEEXT)
	help2man --output=sbsat.1 ./../../src/sbsat$(EXEEXT) 

CONFIG_CLEAN_FILES=sbsat.1
