INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/cudd/include
AM_CFLAGS=-DYYDEBUG
sources=trace_g.yy trace_s.ll \
	aig_g.yy aig_s.ll \
  	nle_g.yy nle_s.ll \
	cnf_g.yy cnf_s.ll \
	bdd_g.yy bdd_s.ll \
	blif_g.yy blif_s.ll \
	prover_g.yy prover_s.ll \
	prover3_g.yy prover3_s.ll \
	iscas_g.yy iscas_s.ll \
        parser.cc functions.cc bddops.cc p3.cc
# _g.yy first
EXTRA_DIST=functions.h bddnode.cc tracetest.cc p3.h 

#BUILT_SOURCES=\
	#libt5_la-aig_g.h  \
	#libt5_la-cnf_g.h  \
	#libt5_la-bdd_g.h \
	#libt5_la-blif_g.h \
	#libt5_la-prover_g.h \
	#libt5_la-prover3_g.h \
	#libt5_la-iscas_g.h \
        #libt5_la-trace_g.cc  \
	#libt5_la-cnf_g.cc  \
	#libt5_la-bdd_g.cc \
	#libt5_la-blif_g.cc \
	#libt5_la-prover_g.cc \
	#libt5_la-prover3_g.cc \
	#libt5_la-iscas_g.cc 

CONFIG_CLEAN_FILES= \
   libt5_a-trace_g.h  \
   libt5_a-aig_g.h  \
   libt5_a-nle_g.h  \
	libt5_a-cnf_g.h  \
	libt5_a-bdd_g.h \
	libt5_a-blif_g.h \
	libt5_a-prover_g.h \
	libt5_a-prover3_g.h \
	libt5_a-iscas_g.h \
	libt5_a-trace_g.cc  \
	libt5_a-aig_g.cc \
 	libt5_a-nle_g.cc \
	libt5_a-cnf_g.cc  \
	libt5_a-bdd_g.cc \
	libt5_a-blif_g.cc \
	libt5_a-prover_g.cc \
	libt5_a-prover3_g.cc \
	libt5_a-iscas_g.cc \
        trace_s.cc  \
	aig_s.cc \
  	nle_s.cc \
	cnf_s.cc  \
	bdd_s.cc \
	blif_s.cc \
	prover_s.cc \
	prover3_s.cc \
	iscas_s.cc \
        trace_s.cc  


#bin_PROGRAMS = tracetest
#tracetest_SOURCES = tracetest.cc  # bddnode.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libt5.la
#libt5_la_SOURCES = $(sources)
#libt5_la_YFLAGS =  -d
#tracetest_LDADD = ../../preproc/libpreproc.la libt5.la ../../utils/libutils.la
#else
noinst_LIBRARIES = libt5.a
libt5_a_SOURCES = $(sources)
libt5_a_YFLAGS =  -d
#tracetest_LDADD = ../../preproc/libpreproc.a libt5.a ../../utils/libutils.a
#endif

#libt5_a_CXXFLAGS =  -pg
#tracetest_CXXFLAGS = -pg
#tracetest_LDFLAGS = -pg

#libt5_a_YFLAGS =  -d
# enable --debug for grammer debugging -- 
# need to take out xxx_debug definitions from parser.c

