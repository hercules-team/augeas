EXTRA_DIST = augtool.pod augparse.pod augmatch.pod augprint.pod

# Generated from the .pod files by pod2man. They are distributed, so that
# building from a release tarball does not require pod2man, and listed in
# CLEANFILES, so that a VPATH build leaves none of them behind in the build
# directory - 'make distcheck' fails on that.
dist_man1_MANS = augtool.1 augparse.1 augmatch.1 augprint.1

CLEANFILES = $(dist_man1_MANS)

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
