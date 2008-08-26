INCLUDES=-I$(top_srcdir)/include
EXTRA_DIST=testite.sh
noinst_LIBRARIES = libgen.a
libgen_a_SOURCES= xorgen.cc vanDerWaerden.cc rn.cc slider2.cc trans.cc rksat.cc \
add_tree.cc #rand_bdd.cc

bin_PROGRAMS = gentest
gentest_SOURCES = gentest.cc
gentest_LDADD = libgen.a #../libsbsat.la $(LIBM)


