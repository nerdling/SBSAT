EXTRA_DIST = BUGS compile.patch  compile.patch.all.sh  compile.patch.sh
# ChangeLog.O README-alpha
SUBDIRS = include src tests examples doc
# doc intl po src tests

if NO_CVSBUILD
distdir=$(PACKAGE)-$(PACKAGE_VERSION)
else
distdir=$(PACKAGE)-$(CVSBUILD)
endif
