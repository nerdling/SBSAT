all: sbsat_getting

edit: sbsat_getting.tex
	ue sbsat_getting.tex

sbsat_getting.dvi: sbsat_getting

sbsat_getting: sbsat_getting.tex
	latex sbsat_getting
	latex sbsat_getting

ps: sbsat_getting
	/usr/bin/dvips sbsat_getting.dvi -o sbsat_getting.ps

pdf: ps
	ps2pdf sbsat_getting.ps

show: sbsat_getting
	p sbsat_getting

clean:
	rm -f *.aux *.log *.dvi *.ps