
EXTRA_DIST=$(wildcard *.pod) $(wildcard *.[0-9])

man1_MANS=augtool.1 augparse.1 augmatch.1 augprint.1

%.1: %.pod
	pod2man -c "Augeas" -r "Augeas $(VERSION)" $< > $@

# The manual pages name the lens directory as an absolute path. What the
# tools actually search is built from $(datadir) by way of DATADIR in
# src/internal.h, so the text is only right when the package is configured
# with --prefix=/usr. Rewrite it on the way in.
install-data-hook:
	@for f in $(man1_MANS); do \
	  p=$(DESTDIR)$(man1dir)/$$f; \
	  sed -e 's|/usr/share/augeas|$(datadir)/augeas|g' $$p > $$p.tmp \
	    && mv -f $$p.tmp $$p; \
	done
