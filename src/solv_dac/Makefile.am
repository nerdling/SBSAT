#SUBDIRS= 

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= dac.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libsolv_dac.la # libsolver2.la
#libsolv_dac_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libsolv_dac.a # libsolver2.la
libsolv_dac_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

