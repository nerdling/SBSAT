INCLUDES=-I$(top_srcdir)/include
SUBDIRS=5
SOURCES=autochk.cc  \
	smurf.cc  bdd2cnf.cc  bddbinary.cc \
	cnf.cc    markbdd.cc xorbdd.cc \
	sat.cc  \
        read_result.cc read_input.cc write_output.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libformats.la
libformats_la_SOURCES=$(SOURCES)
libformats_la_LIBADD=5/libt5.la
else
#noinst_LIBRARIES = libformats.a
#libformats_a_SOURCES=$(SOURCES)
endif
