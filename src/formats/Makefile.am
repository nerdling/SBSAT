INCLUDES=-I$(top_srcdir)/include
SUBDIRS=5
SOURCES=autochk.cc  \
	smurf.cc  bdd2cnf.cc  bddbinary.cc \
	cnf.cc      flatten.cc  markbdd.cc xorbdd.cc \
	parse.cc    sat.cc      tracer.cc   \
        read_result.cc read_input.cc write_output.cc

#if SBSAT_SHARED_BUILD
noinst_LTLIBRARIES = libformats.la
libformats_la_SOURCES=$(SOURCES)
libformats_la_LIBADD=5/libt5.la
#endif

