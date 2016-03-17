GPLOTFILES = temperature_mae.gpi \
	     humidity_mae.gpi

pdf:
	gnuplot $(GPLOTFILES)
	pdflatex report.tex

preview: pdf
	evince report.pdf

clean-intermediate:
	rm -f *.log *.aux

clean:
	rm -f *.log *.aux report.pdf *.png
