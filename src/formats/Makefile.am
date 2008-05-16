INCLUDES=-I$(top_srcdir)/include
SUBDIRS=5
sources=autochk.cc  \
	smurf.cc  bdd2aag.cc bdd2cnf.cc  bddbinary.cc \
	cnf.cc    markbdd.cc xorbdd.cc \
	sat.cc  smurf_fpga.cc\
        read_result.cc read_input.cc write_output.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libformats.la
#libformats_la_SOURCES=$(sources)
#libformats_la_LIBADD=5/libt5.la
#else
noinst_LIBRARIES = libformats.a
libformats_a_SOURCES=$(sources)
#endif

bin_PROGRAMS = aigtoaig
aigtoaig_SOURCES = aigtoaig.c aiger.c aiger.h
