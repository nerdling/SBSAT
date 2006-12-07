INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include

sources= bdd2specfn_and.cc bt_specfn_and.cc fn_and.cc lemmas_and.cc \
	 hr_lsgb_and_init.cc hr_lsgb_and_update.cc \
	 hr_lsgbw_and_init.cc hr_lsgbw_and_update.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnand.la # libsolver2.la
#libfnand_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnand.a # libsolver2.la
libfnand_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
