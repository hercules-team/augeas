
SUBDIRS = naturaldocs

EXTRA_DIST = lenses.tex unambig.tex syntax/augeas.vim ftdetect/augeas.vim bcprules.sty xpath.txt

vimdir = $(datadir)/vim/vimfiles
nobase_vim_DATA = syntax/augeas.vim ftdetect/augeas.vim

# PDF targets
PDFTARGETS=lenses.pdf unambig.pdf

all-local: $(PDFDOCS)

pdfdocs: $(PDFTARGETS)
%.pdf: %.tex
	$(PDFLATEX) $<

clean-local:
	rm -f *.pdf *.aux *.log
