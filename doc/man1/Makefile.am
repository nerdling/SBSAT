dist_man_MANS=sbsat.1
# man1_MANS = sbsat.1

sbsat.1: $(top_srcdir)/src/utils/params.cc $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) -C ../../src 
	@case '$(HELP2MAN)' in \
	  yes) \
	    help2man --output=$(top_builddir)/doc/man1/sbsat.1 $(top_builddir)/src/sbsat$(EXEEXT);; \
	  *) \
	    echo "help2man not installed -- not updating man pages";; \
	esac;

CONFIG_CLEAN_FILES=sbsat.1
