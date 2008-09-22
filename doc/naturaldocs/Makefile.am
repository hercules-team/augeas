
EXTRA_DIST = $(wildcard conf/Augeas.css conf/c_api/*.txt) \
             $(wildcard conf/lenses/*.txt) \
             Modules/NaturalDocs/Languages/Augeas.pm

ND_CONF=$(srcdir)/conf
ND_OUTPUT=output
ND_STYLE=../Augeas

ND_PERL5LIB=$(abs_srcdir)/Modules
ND_PERL5OPT='-MNaturalDocs::Languages::Augeas'

if ND_ENABLED
all-local: NaturalDocs
endif

NaturalDocs: NDLenses NDCAPI

env:
	echo LIB $(ND_PERL5LIB)
	echo OPT $(ND_PERL5OPT)
	test -n "$$PERL5OPT" && ND_PERL5OPT="$(ND_PERL5OPT) $$PERL5OPT" || ND_PERL5OPT=$(ND_PERL5OPT); \
	test -n "$$PERL5LIB" && ND_PERL5LIB="$(ND_PERL5LIB):$$PERL5LIB" || ND_PERL5LIB=$(ND_PERL5LIB); \
        PERL5LIB=$$ND_PERL5LIB PERL5OPT=$$ND_PERL5OPT env | grep PERL

NDLenses: NDConf
	@mkdir -p $(ND_OUTPUT)/lenses
	@(echo "Format lens documentation"; \
	  test -n "$$PERL5OPT" && ND_PERL5OPT="$(ND_PERL5OPT) $$PERL5OPT" || ND_PERL5OPT=$(ND_PERL5OPT); \
	  test -n "$$PERL5LIB" && ND_PERL5LIB="$(ND_PERL5LIB):$$PERL5LIB" || ND_PERL5LIB=$(ND_PERL5LIB); \
	  PERL5LIB=$$ND_PERL5LIB PERL5OPT=$$ND_PERL5OPT \
          $(ND_PROG) -p conf/lenses \
	      -i $(top_srcdir)/lenses \
	      -o $(ND_FORMAT) $(ND_OUTPUT)/lenses \
	      -s $(ND_STYLE))

NDCAPI: NDConf
	@mkdir -p $(ND_OUTPUT)/c_api
	@(echo "Format C API documentation"; \
	  test -n "$$PERL5OPT" && ND_PERL5OPT="$(ND_PERL5OPT) $$PERL5OPT" || ND_PERL5OPT=$(ND_PERL5OPT); \
	  test -n "$$PERL5LIB" && ND_PERL5LIB="$(ND_PERL5LIB):$$PERL5LIB" || ND_PERL5LIB=$(ND_PERL5LIB); \
	  $(ND_PROG) -p conf/c_api \
	    -i $(top_srcdir)/src \
	    -o $(ND_FORMAT) $(ND_OUTPUT)/c_api \
	    -s $(ND_STYLE))

NDConf:
	@(if test ! -d $(ND_CONF); then \
	    cp -pr $(ND_CONF) conf; \
	  fi)

clean-local:
	rm -rf output conf/Data
	rm -rf $(ND_CONF)/c_api/Data $(ND_CONF)/lenses/Data
