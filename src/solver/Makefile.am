INCLUDES=-I$(top_srcdir)/include
noinst_LIBRARIES = libsolver.a libsolver2.a
EXTRA_SOURCE=heur_choice.cc

libsolver_a_SOURCES= solve.cc bddwalk.cc \
             init_solver.cc brancher.cc \
	     backtrack.cc backtrack_nl.cc backtrack_sbj.cc bt_misc.cc \
             select_bp.cc \
             j_heuristic.cc j_smurf.cc j_specfn.cc j_update_heu.cc \
             l_heuristic.cc l_lemma.cc \
	     i_heuristic.cc \
             heuristic.cc \
             bt_lemmas.cc bt_specfn.cc bt_smurfs.cc \
             bt_specfn_and.cc bt_specfn_xor.cc \
             lemmainfo.cc lemmaspace.cc smurffactory.cc smurfstates.cc \
             state_stacks.cc \
	     bdd2smurf.cc specfn2smurf.cc autarky.cc \
             display.cc display_sf.cc verify.cc \
	     recordsol.cc sf_addons.cc transitions.cc wvf.cc update_heu.cc \
	     crtwin.cc load_lemmas.cc interface.cc heur_stack.cc

libsolver2_a_SOURCES=libsolver.cc

