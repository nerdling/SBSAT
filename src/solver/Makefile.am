INCLUDES=-I$(top_srcdir)/include
SOURCES= solve.cc ksolve.cc \
             init_solver.cc brancher.cc \
	     backtrack.cc backtrack_nl.cc backtrack_sbj.cc bt_misc.cc \
             select_bp.cc \
             j_heuristic.cc j_smurf.cc j_update_heu.cc \
	     j_specfn.cc j_specfn_and.cc j_specfn_xor.cc j_specfn_minmax.cc \
	     l_heuristic.cc l_lemma.cc \
	     i_heuristic.cc \
             heuristic.cc \
             bt_lemmas.cc bt_specfn.cc bt_smurfs.cc \
             bt_specfn_and.cc bt_specfn_xor.cc bt_specfn_minmax.cc \
             lemmas.cc lemmas_and.cc lemmas_xor.cc lemmas_minmax.cc \
	     lemmacache.cc lemmainfo.cc lemmaspace.cc lemmawlits.cc lemmamisc.cc \
             smurffactory.cc smurfstates.cc \
             state_stacks.cc \
	     bdd2smurf.cc bdd2specfn_and.cc bdd2specfn_xor.cc bdd2specfn_minmax.cc \
	     autarky.cc \
	     display.cc display_sf.cc verify.cc \
	     recordsol.cc sf_addons.cc smurfaddons.cc transitions.cc \
	     update_heu.cc \
	     crtwin.cc load_lemmas.cc interface.cc heur_stack.cc \
	     bddwalk.cc wvf.cc 
EXTRA_DIST=heur_choice.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libsolver.la libsolver2.la
libsolver_la_SOURCES= $(SOURCES)
libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif
