INCLUDES=-I$(top_srcdir)/include
EXTRA_DIST=testite.sh
noinst_LIBRARIES = libgen.a
libgen_a_SOURCES= xorgen.cc vanDerWaerden.cc rn.cc slider2.cc

bin_PROGRAMS = gentest
gentest_SOURCES = gentest.cc
gentest_LDADD = libgen.a


