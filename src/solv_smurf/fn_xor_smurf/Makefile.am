INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
SOURCES = fn_xor_smurf.cc \
	  hr_lsgb_xorsmurf.cc \
	  #hr_lsgbw_xor.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libfnxorsmurf.la # libsolver2.la
libfnxorsmurf_la_SOURCES= $(SOURCES)
#libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif

