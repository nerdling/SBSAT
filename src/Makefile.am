INCLUDES=-I$(top_srcdir)/include
bin_PROGRAMS = ite
ite_SOURCES = ite.cc
ite_LDADD = formats/libformats.a \
	    formats/5/libt5.a \
	    solver/libsolver2.a  \
	    postproc/libpostproc.a \
	    preproc/libpreproc.a \
            utils/libutils.a \
	    $(LIBM)

#            formats/5/libt5.a 
SUBDIRS = solver utils preproc postproc formats generator
