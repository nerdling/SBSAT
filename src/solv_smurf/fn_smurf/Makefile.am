INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= bdd2smurf.cc transitions.cc smurfstates.cc fn_smurf.cc bt_smurf.cc \
	 hr_lsgb_smurf_update.cc  hr_lsgb_smurf_init.cc \
	 hr_lsgbw_smurf_update.cc  hr_lsgbw_smurf_init.cc 

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnsmurf.la # libsolver2.la
#libfnsmurf_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnsmurf.a # libsolver2.la
libfnsmurf_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

