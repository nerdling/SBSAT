INCLUDES=-I$(top_srcdir)/include
AM_CFLAGS=-DYYDEBUG
EXTRA_DIST=bddnode.h functions.h bddnode.cc tracetest.cc
noinst_LIBRARIES = libt5.a
#libgen_a_SOURCES= xorgen.cc

libt5_a_SOURCES = trace_g.yy cnf_g.yy bdd_g.yy trace_s.ll cnf_s.ll bdd_s.ll parser.cc functions.cc bddops.cc blif_g.yy blif_s.ll
#libt5_a_YFLAGS =  -o y.tab.c -d -y 
libt5_a_YFLAGS =  -d 

bin_PROGRAMS = tracetest
tracetest_SOURCES = tracetest.cc  # bddnode.cc
#tracetest_LDADD = libt5.a ../../preproc/libpreproc.a ../../utils/libutils.a
tracetest_LDADD = ../../preproc/libpreproc.a libt5.a ../../utils/libutils.a



#libt5_a_CXXFLAGS =  -pg
#tracetest_CXXFLAGS = -pg
#tracetest_LDFLAGS = -pg

