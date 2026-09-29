EXTRA_DIST = augtool.pod augparse.pod augmatch.pod augprint.pod

# Generated from the .pod files by pod2man. They are distributed, so that
# building from a release tarball does not require pod2man, and listed in
# CLEANFILES, so that a VPATH build leaves none of them behind in the build
# directory - 'make distcheck' fails on that.
dist_man1_MANS = augtool.1 augparse.1 augmatch.1 augprint.1

CLEANFILES = $(dist_man1_MANS)

%.1: %.pod
	pod2man -c "Augeas" -r "Augeas $(VERSION)" $< > $@
