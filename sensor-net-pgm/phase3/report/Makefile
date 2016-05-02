GPLOTFILES = phase1-files/temperature_mae.gpi \
	     phase1-files/humidity_mae.gpi

pdf:
	gnuplot $(GPLOTFILES)
	pdflatex report.tex

preview: pdf
	evince report.pdf

clean-intermediate:
	rm -f *.log *.aux

clean:
	rm -f *.log *.aux report.pdf phase1-files/*.png phase2-files/*.png *.png
