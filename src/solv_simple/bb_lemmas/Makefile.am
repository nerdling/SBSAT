INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_simple/include

EXTRA_DIST= picosat.h

sources= picosat.c

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnand.la # libsolver2.la
#libfnand_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libbblemmas_simple.a # libsolver2.la
libbblemmas_simple_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
