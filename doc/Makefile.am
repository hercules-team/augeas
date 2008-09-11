
SUBDIRS = $(HAS_NATURALDOCS)

# PDF targets
PDFTARGETS=lenses.pdf unambig.pdf

all: $(PDFDOCS)

pdfdocs: $(PDFTARGETS)
%.pdf: %.tex
	$(PDFLATEX) $<

clean-local:
	rm -f *.pdf *.aux *.log

