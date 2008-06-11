INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_simple/include

sources= fn_negminmax.cc bdd2specfn_negminmax.cc bt_specfn_negminmax.cc hr_lsgb_negminmax_init.cc \
         hr_lsgb_negminmax_update.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnand.la # libsolver2.la
#libfnand_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnnegminmax_simple.a # libsolver2.la
libfnnegminmax_simple_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
