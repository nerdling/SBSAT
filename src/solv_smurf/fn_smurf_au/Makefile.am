INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
SOURCES= bdd2smurf_au.cc bt_smurf_au.cc fn_smurf_au.cc smurfstates_au.cc transitions_au.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libfnsmurfau.la # libsolver2.la
libfnsmurfau_la_SOURCES= $(SOURCES)
#libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif

