# Makefile for the 'doc' directory of the btparse distribution

include ../Makefile.defs        # for installation dirs
include ../Makefile.version	# get VERSION and RELEASE_DATE macros


POD = bt_input.pod				\
      bt_language.pod				\
      bt_traversal.pod				\
      btparse.pod

MAN3 = $(POD:.pod=.3)

.SUFFIXES : .pod .3

.pod.3:
	pod2man --section=3 \
                --release="btparse, version $(VERSION)" \
                --center="btparse" \
                --date="$(RELEASE_DATE)" \
                $< > $@

man : $(MAN3)

clean :
	rm -f $(MAN3)

scrub :      # dummy target (used by ../Makefile)

install : man
	cp $(MAN3) $(INSTALL_MAN3)
