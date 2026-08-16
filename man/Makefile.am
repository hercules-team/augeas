
EXTRA_DIST=augtool.pod augparse.pod augmatch.pod augprint.pod $(man1_MANS)

man1_MANS=augtool.1 augparse.1 augmatch.1 augprint.1

%.1: %.pod
	pod2man -c "Augeas" -r "Augeas $(VERSION)" $< > $@
