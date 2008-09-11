
NDPROJECT=$(srcdir)/conf
NDSRC=$(top_srcdir)/src
NDLENSES=$(top_srcdir)/lenses
NDFORMAT=HTML
NDOUTPUT=output
NDLOADMODULES=NaturalDocs::Languages::Augeas
NDMODULES=$(NATURALDOCSDIR)/Modules
NDLOCALMODULES=$(srcdir)/Modules
NDSTYLE=Augeas

all: NaturalDocs

NaturalDocs: NDLenses NDCAPI

NDLenses:
	mkdir -p $(NDOUTPUT)/lenses
	exec $(PERL) \
	  -M"$(NDLOADMODULES)" \
	  -I"$(NDMODULES)" \
	  -I"$(NDLOCALMODULES)" \
	  $(NATURALDOCSDIR)/NaturalDocs \
	    -p $(NDPROJECT)/lenses \
	    -i $(NDLENSES) \
	    -o $(NATURALDOCSFORMAT) $(NDOUTPUT)/lenses \
	    -s $(NDSTYLE)

NDCAPI:
	mkdir -p $(NDOUTPUT)/c_api
	exec $(PERL) \
	  $(NATURALDOCSDIR)/NaturalDocs \
	    -p $(NDPROJECT)/c_api \
	    -i $(NDSRC) \
	    -o $(NATURALDOCSFORMAT) $(NDOUTPUT)/c_api \
	    -s $(NDSTYLE)

clean-local:
	rm -rf output conf/Data
	rm -rf $(NDPROJECT)/c_api/Data $(NDPROJECT)/lenses/Data

