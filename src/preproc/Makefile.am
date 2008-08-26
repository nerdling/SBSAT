INCLUDES=-I$(top_srcdir)/include
sources= create_infs.cc do_cofactor.cc   do_exquantify.cc   \
	do_strength.cc other_preproc.cc do_applyinfs.cc do_depcluster.cc  \
	do_exquantifyand.cc  do_pruning.cc init_preproc.cc \
	preprocess.cc do_pruning_1.cc do_pruning_2.cc      \
	do_pruning_restrict.cc do_flow.cc do_pruning_pr.cc \
	do_split.cc do_steal.cc do_rewind.cc do_clearft.cc \
	do_prover3.cc equivclass.cc do_simpleand.cc \
	do_possibleanding.cc do_findft.cc do_safeassign.cc \
	do_safesearch.cc do_ex_safe_cluster.cc do_identify_same_structure.cc \
	do_extendres.cc do_diameter.cc do_message_passing.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libpreproc.la
#libpreproc_la_SOURCES=$(sources)
#else
noinst_LIBRARIES = libpreproc.a
libpreproc_a_SOURCES=$(sources)
#endif
