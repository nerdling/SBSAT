#SUBDIRS= 

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= bddwalk.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libsolv_bddwalk.la # libsolver2.la
#libsolv_bddwalk_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libsolv_bddwalk.a # libsolver2.la
libsolv_bddwalk_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

