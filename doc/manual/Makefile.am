EXTRA_DIST=README sbsat_manual.lyx sbsat_manual.ps sbsat_manual.pdf sbsat_manual.html sbsat_manual.tex sbsat_getting.lyx sbsat_getting.pdf

sbsat_manual.pdf: sbsat_manual.lyx
	lyx -e pdf sbsat_manual.lyx

sbsat_manual.html: sbsat_manual.lyx
	lyx -e html sbsat_manual.lyx

sbsat_manual.ps: sbsat_manual.lyx
	lyx -e ps sbsat_manual.lyx

sbsat_manual.tex: sbsat_manual.lyx
	lyx -e latex sbsat_manual.lyx

sbsat_getting.pdf: sbsat_getting.lyx
	lyx -e pdf sbsat_getting.lyx

