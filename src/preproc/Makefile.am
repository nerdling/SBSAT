INCLUDES=-I$(top_srcdir)/include
SOURCES= create_infs.cc do_cofactor.cc   do_exquantify.cc   \
	do_strength.cc other_preproc.cc do_applyinfs.cc do_depcluster.cc  \
	do_exquantifyand.cc  do_pruning.cc init_preproc.cc \
	preprocess.cc do_pruning_1.cc do_pruning_2.cc      \
	do_pruning_restrict.cc do_flow.cc do_pruning_pr.cc \
	do_split.cc do_steal.cc do_rewind.cc do_clearft.cc \
	do_prover3.cc equivclass.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libpreproc.la
libpreproc_la_SOURCES=$(SOURCES)
else
#noinst_LIBRARIES = libpreproc.a
#libpreproc_a_SOURCES=$(SOURCES)
endif
