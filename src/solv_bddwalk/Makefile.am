#SUBDIRS= 

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
SOURCES= bddwalk.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libsolv_bddwalk.la # libsolver2.la
libsolv_bddwalk_la_SOURCES= $(SOURCES)
#libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif

