SUBDIRS= include fn_lemma_au fn_smurf_au hr_lsgbw hr_lemma fn_minmax fn_xor \
fn_and fn_lemma fn_smurf hr_null hr_lsgb hr_lsgb_lemma hr_vsids hr_berkmin \
hr_anne hr_minisat fn_smurf_xor fn_xor_smurf
#fn_plainor fn_smurf_au 

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= display.cc solver.cc \
	 fn_init.cc fn_create.cc fn_stack.cc fn_update.cc \
	 vars_inferlit.cc vars_recordsol.cc vars_verify.cc vars_map.cc vars_init.cc vars_hints.cc \
	 br_init.cc br_afs.cc br_checks.cc br_brancher.cc br_select_bp.cc br_backtrack_nl.cc br_backtrack.cc
#	heur_i.cc heur_j.cc 
#	heur_j_update.cc heur_l.cc heur_null.cc heur_stack.cc heur_update.cc \
#	libsolver.cc 
# display.cc display_sf.cc
# backtrack_nl.cc backtrack_sbj.cc 
#
EXTRA_DIST=hr_choice.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libsolv_smurf.la # libsolver2.la
#libsolv_smurf_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libsolv_smurf.a # libsolver2.la
libsolv_smurf_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

