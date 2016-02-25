generate-pdf:
	pdflatex paper.tex

run-xdvi: compile
	xdvi paper.dvi

compile:
	latex paper.tex

clean:
	rm -rf *.aux *.log *.log *.dvi *.pdf
