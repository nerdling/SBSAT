all: sbsat_user_manual

edit: sbsat_user_manual.tex
	ue sbsat_user_manual.tex

sbsat_user_manual.dvi: sbsat_user_manual

sbsat_user_manual: sbsat_user_manual.tex
	latex sbsat_user_manual
	latex sbsat_user_manual

ps: sbsat_user_manual
	 dvips sbsat_user_manual.dvi -o sbsat_user_manual.ps

pdf: ps
	ps2pdf sbsat_user_manual.ps

show: sbsat_user_manual
	p sbsat_user_manual

clean:
	rm -f *.aux *.log *.dvi *.ps *.toc *.pdf
