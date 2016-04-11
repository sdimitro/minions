GROFF_SRC=cc_project.ms
GROFF_OUT=Proj_Dimitropoulos-Serapheim.ps
GROFF_PDF=Proj_Dimitropoulos-Serapheim.pdf
PS_VIEWER=evince
PS2PDF=ps2pdf

pdf: compile
	$(PS2PDF) $(GROFF_OUT)


preview: compile
	evince $(GROFF_OUT)

compile:
	groff -m ms $(GROFF_SRC) > $(GROFF_OUT)

clean:
	rm -f $(GROFF_OUT) $(GROFF_PDF)
