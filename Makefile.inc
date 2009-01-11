# -*- Makefile-automake -*-
#
# Support for running programs with failmalloc preloaded. Include in other
# automake files and make sure the following variables are set:
#
# FAILMALLOC_START - number of first FAILMALLOC_INTERVAL
# FAILMALLOC_REP   - how often to repeat with increasing FAILMALLOC_INTERVAL
# FAILMALLOC_PROG  - the program to run with linfailmalloc preloaded

if WITH_FAILMALLOC
failmalloc: failmalloc-run
else
failmalloc: failmalloc-error
endif

failmalloc-run: $(FAILMALLOC_PROG)
	@(echo "Running $(FAILMALLOC_PROG) with failmalloc";    \
      for i in $$(seq $(FAILMALLOC_START) $$(expr $(FAILMALLOC_START) + $(FAILMALLOC_REP) - 1)) ; do \
	  resp=$$(libtool --mode=execute env LD_PRELOAD=$(LIBFAILMALLOC) FAILMALLOC_INTERVAL=$$i $(FAILMALLOC_PROG));        \
	  status=$$?;                                          \
	  if [ $$status -ne 0 -a $$status -ne 2 ] ;	then       \
	    printf "%5d FAIL %3d %s\n" $$i $$status "$$resp" ; \
	  elif [ x$(V) = x1 -o $$(( $$i % 100 )) -eq 0 ] ; then \
	    printf "%5d PASS %s\n" $$i "$$resp" ;              \
	  fi                                                   \
	done)

failmalloc-error:
	@(echo "You need to turn on failmalloc support with --with-failmalloc"; \
	  exit 1)
