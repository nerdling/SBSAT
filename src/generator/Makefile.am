INCLUDES=-I$(top_srcdir)/include
EXTRA_DIST=testite.sh slider3_base.py
noinst_LIBRARIES = libgen.a
libgen_a_SOURCES= xorgen.cc vanDerWaerden.cc rn.cc slider2.cc trans.cc \
rksat.cc add_tree.cc #rand_bdd.cc

sed -i "s/__PREFIX__/${prefix}/" gentest.cc

bin_PROGRAMS = gentest
gentest_SOURCES = gentest.cc
gentest_LDADD = libgen.a #../libsbsat.la $(LIBM)

bin_SCRIPTS = slider3.py
CLEANFILES = $(bin_SCRIPTS)

slider3.py: slider3_base.py
	rm -f slider3.py
	echo "#! " $(PYTHON) > slider3.py
	cat slider3_base.py >> slider3.py
	chmod +x slider3.py
