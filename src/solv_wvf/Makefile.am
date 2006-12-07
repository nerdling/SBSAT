#SUBDIRS= 

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= wvf.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libsolv_wvf.la # libsolver2.la
#libsolv_wvf_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libsolv_wvf.a # libsolver2.la
libsolv_wvf_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

