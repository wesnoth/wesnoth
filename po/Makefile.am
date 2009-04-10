SUBDIRS = wesnoth wesnoth-anl wesnoth-aoi wesnoth-did wesnoth-dm wesnoth-editor wesnoth-ei wesnoth-httt wesnoth-l wesnoth-lib wesnoth-low wesnoth-multiplayer wesnoth-nr wesnoth-sof wesnoth-sotbe wesnoth-tb wesnoth-test wesnoth-thot wesnoth-trow wesnoth-tsg wesnoth-tutorial wesnoth-units wesnoth-utbs

PO4A_SUBDIRS = wesnoth-manpages
PO4A_OPTIONS = --rm-backups --copyright-holder "Wesnoth Development Team"

EXTRA_DIST = Makefile.in.in

update-po:
	@for dir in $(SUBDIRS); do \
		( cd $$dir && make $@ || exit $? ); \
	done
if PO4AUPDATE
	@for dir in $(PO4A_SUBDIRS); do \
		( cd $$dir && $(PO4A) $(PO4A_OPTIONS) --no-translations $$dir.cfg || \
		  exit $? ); \
	done
endif

if PO4AUPDATE
update-po4a:
	@for dir in $(PO4A_SUBDIRS); do \
		( cd $$dir && $(PO4A) $(PO4A_OPTIONS) $$dir.cfg || exit $? ); \
	done
endif

distclean-local:
	@for dir in $(SUBDIRS); do \
		rm -f $$dir/Makefile.in.in ; \
	done
