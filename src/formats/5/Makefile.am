INCLUDES=-I$(top_srcdir)/include
AM_CFLAGS=-DYYDEBUG
SOURCES=trace_g.yy trace_s.ll \
	cnf_g.yy cnf_s.ll \
	bdd_g.yy bdd_s.ll \
	blif_g.yy blif_s.ll \
	prover_g.yy prover_s.ll \
	prover3_g.yy prover3_s.ll \
	iscas_g.yy iscas_s.ll \
        parser.cc functions.cc bddops.cc p3.cc
# _g.yy first
EXTRA_DIST=bddnode.h functions.h bddnode.cc tracetest.cc p3.h \
        libt5_la-trace_g.h  \
	libt5_la-cnf_g.h  \
	libt5_la-bdd_g.h \
	libt5_la-blif_g.h \
	libt5_la-prover_g.h \
	libt5_la-prover3_g.h \
	libt5_la-iscas_g.h

SAVE4LATER = \
       libt5_a-trace_g.h  \
	libt5_a-cnf_g.h  \
	libt5_a-bdd_g.h \
	libt5_a-blif_g.h \
	libt5_a-prover_g.h \
	libt5_a-prover3_g.h \
	libt5_a-iscas_g.h 


bin_PROGRAMS = tracetest
tracetest_SOURCES = tracetest.cc  # bddnode.cc

libt5_la_YFLAGS =  -d
if MYLIBTOOL
noinst_LTLIBRARIES = libt5.la
libt5_la_SOURCES = $(SOURCES)
tracetest_LDADD = ../../preproc/libpreproc.la libt5.la ../../utils/libutils.la
else
#noinst_LIBRARIES = libt5.a
#libt5_a_SOURCES = $(SOURCES)
#libt5_a_YFLAGS =  -d
#tracetest_LDADD = ../../preproc/libpreproc.a libt5.a ../../utils/libutils.a
endif

#libt5_a_CXXFLAGS =  -pg
#tracetest_CXXFLAGS = -pg
#tracetest_LDFLAGS = -pg

#libt5_a_YFLAGS =  -d
# enable --debug for grammer debugging -- 
# need to take out xxx_debug definitions from parser.c

