
SUBDIRS = naturaldocs

EXTRA_DIST = lenses.tex unambig.tex

# PDF targets
PDFTARGETS=lenses.pdf unambig.pdf

all-local: $(PDFDOCS)

pdfdocs: $(PDFTARGETS)
%.pdf: %.tex
	$(PDFLATEX) $<

clean-local:
	rm -f *.pdf *.aux *.log
